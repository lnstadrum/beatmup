#include "sequence.h"
#include "../exception.h"
#include <algorithm>

using namespace Beatmup::Fragments;

#ifdef BEATMUP_DEBUG
void consistencyTest(const std::vector<FragmentPtr>& fragments) {
	int index = 0;
	for (auto it : fragments) {
		if (it.length <= 0)
			BEATMUP_ERROR("Non-positive length in fragment %d: %d", index, it.length);
		if (it.offset < 0)
			BEATMUP_ERROR("Negative offset in fragment %d: %d", index, it.offset);
		if (it.length + it.offset > it.fragment->getSampleCount())
			BEATMUP_ERROR("Out-of-range mapping of fragment %d: %d", index, it.fragment->getSampleCount());
		index++;
	}
}
#endif

Sequence::Sequence() {
	cumTimes.push_back(0);
}


Sequence::~Sequence() {
	if (pointers.size() > 0)
		BEATMUP_ERROR("Deleting pointed sequence");
}


int Sequence::findFragment(int time) const {
	int index = fragments.size() / 2, step = std::max(index / 2, 1);
	
	// simple log search
	do {
		if (cumTimes[index] > time)
			index -= step;
		else
			if (time >= cumTimes[index+1])
				index += step;
			else
				// target fragment reached
				return index;

		// halfing the step
		step = std::max(step / 2, 1);
	} while (0 <= index && index < fragments.size());

	// no fragment found, the index is outside of the sequence scope
	if (index < 0)
		return VOID_LEFT;
	return VOID_RIGHT;
}


void Sequence::splitFragment(int index, int dt) {
	if (dt > 0) {
		fragments.insert(fragments.begin() + index, fragments[index]);
		fragments[index].length = dt;
		fragments[index + 1].length -= dt;
		fragments[index + 1].offset += dt;
	}
}


void Sequence::concatenate(Fragment& fragment, int offset, int length) {
	fragments.emplace_back(fragment, offset, length);
	cumTimes.push_back(cumTimes.back() + length);
}


void Sequence::syncPointers() {
	for (auto ptr : pointers)
		ptr->moveTo(ptr->getTime());
}


void Sequence::clear() {
	fragments.clear();
	cumTimes.clear();
	cumTimes.push_back(0);
	syncPointers();
}


void Sequence::shrink(int timeLeft, int timeRight) {
	if (timeLeft >= timeRight || timeLeft >= getLength() || timeRight <= 0) {
		clear();
		return;
	}
	int
		leftFragment = findFragment(timeLeft),
		rightFragment = findFragment(timeRight);

	// FIXME: time bounds can fall out of the sequence scope
	BEATMUP_ASSERT_DEBUG(leftFragment >= 0 && rightFragment >= 0);

	// right bound may generate an empty fragment
	if (cumTimes[rightFragment] == timeRight)
		--rightFragment;

	// shrinking fragment list if needed
	if (rightFragment != fragments.size() - 1)
		fragments.erase(fragments.begin() + rightFragment + 1, fragments.end());
	if (leftFragment > 0)
		fragments.erase(fragments.begin(), fragments.begin() + leftFragment);

	// adjust time bounds
	int dt = timeLeft - cumTimes[leftFragment];
	fragments[0].offset += dt;
	fragments[0].length -= dt;
	fragments.back().length = timeRight - cumTimes[rightFragment];

	// recompute cumulative time index
	cumTimes.clear();
	cumTimes.reserve(fragments.size() + 1);
	cumTimes.push_back(0);

	// special case: single fragment left after shrinking
	if (leftFragment == rightFragment) {
		dt = timeRight - timeLeft;
		cumTimes.push_back(dt);
		fragments.back().length = dt;
	}
	else {
		int time = 0;
		for (const auto &it : fragments)
			cumTimes.push_back(time += it.length);
	}

	// synchronize pointers
	syncPointers();

#ifdef BEATMUP_DEBUG
	consistencyTest(fragments);
#endif
}


Sequence* Sequence::copy(int fromTime, int toTime) const {
	if (fromTime >= toTime)
		return NULL;		// nothing to copy
	
	int
		fromFragment = findFragment(fromTime),
		toFragment = findFragment(toTime);
	
	// whole sequence is apart
	if ((toFragment == VOID_LEFT || toFragment == VOID_RIGHT) && fromFragment == toFragment)
		return NULL;

	// now we are sure that there is some data to copy
	if (fromFragment == VOID_LEFT || toFragment == VOID_RIGHT)
		throw Sequence::AccessException("A border is out of sequence scope", *this);

	// right bound may generate an empty fragment
	if (cumTimes[toFragment] == toTime)
		--toFragment;

	Sequence* result = createEmpty();
	int numFragments = toFragment - fromFragment + 1;
	result->fragments.reserve(numFragments);
	result->cumTimes.clear();
	result->cumTimes.reserve(numFragments + 1);
	result->cumTimes.push_back(0);

	// put first fragment
	int dt = fromTime - cumTimes[fromFragment];
	result->fragments.emplace_back(fragments[fromFragment]);
	result->fragments[0].offset += dt;

	// special case: single fragment copied
	if (fromFragment == toFragment) {
		dt = toTime - fromTime;
		result->fragments[0].length = dt;
		result->cumTimes.push_back(dt);
	}
	else {
		int cumTime = (result->fragments[0].length -= dt);
		result->cumTimes.push_back(cumTime);

		// go
		for (int i = fromFragment + 1; i <= toFragment - 1; ++i) {
			// putting new fragment
			result->fragments.emplace_back(fragments[i]);
			cumTime += fragments[i].length;
			result->cumTimes.push_back(cumTime);
		}

		// right boundary
		result->fragments.emplace_back(fragments[toFragment]);
		int l = toTime - cumTimes[toFragment];
		result->fragments.back().length = l;
		result->cumTimes.push_back(cumTime + l);
	}
	
#ifdef BEATMUP_DEBUG
	consistencyTest(fragments);
#endif
	return result;
}


void Sequence::insert(const Sequence& sequence, int time) {
	// check if the time is okay
	if (time < 0 || time > getLength())		// time may be equal to the length
		throw AccessException("Bad insert position", *this);

	// find the fragment (guaranteeing that it is split it in two)
	int index = findFragment(time);
	if (time > cumTimes[index]) {
		int dt = time - cumTimes[index];
		splitFragment(index, dt);
		index++;
		cumTimes[index] = cumTimes[index - 1] + dt;
	}

	// insert remaining part of the sequence
	fragments.insert(fragments.begin() + index, sequence.fragments.begin(), sequence.fragments.end());
	cumTimes.resize(fragments.size() + 1);
	int cumTime = cumTimes[index];
	for (auto it = fragments.begin() + index; it != fragments.end(); ++it)
		cumTimes[++index] = (cumTime += it->length);

	syncPointers();
#ifdef BEATMUP_DEBUG
	consistencyTest(fragments);
#endif
}


void Sequence::remove(int fromTime, int toTime) {
	if (fromTime >= toTime)
		throw AccessException("Inconsistent time bounds when removing", *this);

	// nothing to remove
	if (toTime < 0 || getLength() <= fromTime)
		return;

	// whole sequence erased
	if (fromTime <= 0 || toTime > getLength()) {
		clear();
		return;
	}

	int
		fromFragment = fromTime >= cumTimes[1] ? findFragment(fromTime) : 0,
		toFragment = findFragment(toTime);

	// determine fragment indices range to remove
	int dt = fromTime - cumTimes[fromFragment];
	int removeRangeBegin = fromFragment;
	if (dt > 0)
		removeRangeBegin++;
	int removeRangeEnd = toFragment >= 0 ? toFragment : fragments.size();
	
	// remove fragments
	if (removeRangeBegin <= removeRangeEnd) {
		if (removeRangeBegin < removeRangeEnd)
			fragments.erase(fragments.begin() + removeRangeBegin, toFragment >= 0 ? fragments.begin() + toFragment : fragments.end());

		// adjust boundary fragments timings
		int index = fromFragment;
		if (dt > 0) {
			fragments[index].length = dt;
			index++;
		}
		if (toFragment >= 0) {
			dt = toTime - cumTimes[toFragment];
			fragments[index].offset += dt;
			fragments[index].length -= dt;
		}
	}
	// special case: single fragment concerned
	else {
		int dt = fromTime - cumTimes[fromFragment];
		splitFragment(fromFragment, dt);
		dt = toTime - fromTime;
		int index = fromFragment + 1;
		fragments[index].offset += dt;
		fragments[index].length -= dt;
	}

	// update cumulative time index
	cumTimes.resize(fragments.size() + 1);
	int index = fromFragment;
	int cumTime = cumTimes[index];
	for (auto it = fragments.begin() + index; it != fragments.end(); ++it)
		cumTimes[++index] = (cumTime += it->length);
	
	syncPointers();

#ifdef BEATMUP_DEBUG
	consistencyTest(fragments);
#endif
}


void Sequence::split(int time, Sequence* left, Sequence* right) {
	const int L = getLength();
	if (time <= 0) {
		left = NULL;
		right = this;
	}
	else if (time >= L-1) {
		left = this;
		right = NULL;
	}
	else {
		left = this;
		right = copy(time, L);
		shrink(0, time);
	}
}



Sequence::Pointer::Pointer(Sequence& sequence, int time, bool writing) : sequence(sequence), writing(writing), watching(false) {
	if (0 <= time && time < sequence.cumTimes[1]) {
		// an heuristic to start quickly
		if (writing)
			sequence.fragments[0].editData();
		currentTime = time;
		pointer = sequence.fragments[0];
		pointer.offset += time;
		pointer.length -= time;
		fragmentIdx = 0;
	}
	else
		moveTo(time);
}


Sequence::Pointer::~Pointer() {
	if (watching)
		sequence.pointers.erase(std::find(sequence.pointers.begin(), sequence.pointers.end(), this));
}


void Sequence::Pointer::moveTo(int time) {
	currentTime = time;
	fragmentIdx = sequence.findFragment(time);
	if (fragmentIdx != VOID_LEFT && fragmentIdx != VOID_RIGHT) {
		if (writing)
			sequence.fragments[fragmentIdx].editData();
		pointer = sequence.fragments[fragmentIdx];
		int dt = time - sequence.cumTimes[fragmentIdx];
		pointer.offset += dt;
		pointer.length -= dt;
	}
	else
		pointer.nullify();
}


void Sequence::Pointer::step() {
	if (0 <= fragmentIdx && fragmentIdx < sequence.fragments.size()-1) {
		fragmentIdx++;
		pointer = sequence.fragments[fragmentIdx];
		currentTime = sequence.cumTimes[fragmentIdx];
	}
	else {
		fragmentIdx = VOID_RIGHT;
		pointer.nullify();
		currentTime = sequence.getLength();
	}
}


void Sequence::Pointer::watch() {
	if (!watching) {
		watching = true;
		sequence.pointers.push_back(this);
	}
}
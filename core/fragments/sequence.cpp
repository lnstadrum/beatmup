#include "sequence.h"
#include "../exception.h"
#include "../debug.h"
#include <algorithm>

using namespace Beatmup::Fragments;

#ifdef BEATMUP_DEBUG
void consistencyTest(const std::vector<FragmentPtr>& fragments) {
    int index = 0;
    for (auto it : fragments) {
        Beatmup::DebugAssertion::check(it.length > 0,
            "Non-positive length in fragment %d: %d", index, it.length);
        Beatmup::DebugAssertion::check(it.offset >= 0,
            "Negative offset in fragment %d: %d", index, it.offset);
        Beatmup::DebugAssertion::check(it.length + it.offset <= it.fragment->getSampleCount(),
            "Out-of-range mapping of fragment %d: %d", index, it.fragment->getSampleCount());
        index++;
    }
}
#endif

Sequence::Sequence() {
    cumTimes.push_back(0);
}


Sequence::~Sequence() {
    if (pointers.size() > 0)
        BEATMUP_DEBUG_E("Deleting pointed sequence");
}


int Sequence::findFragment(dtime time) const {
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


void Sequence::splitFragment(int index, dtime dt) {
    if (dt > 0) {
        fragments.insert(fragments.begin() + index, fragments[index]);
        fragments[index].length = dt;
        fragments[index + 1].length -= dt;
        fragments[index + 1].offset += dt;
    }
}


void Sequence::concatenate(Fragment& fragment, dtime offset, dtime length) {
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


void Sequence::shrink(dtime timeLeft, dtime timeRight) {
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
    dtime dt = timeLeft - cumTimes[leftFragment];
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
        dtime time = 0;
        for (const auto &it : fragments)
            cumTimes.push_back(time += it.length);
    }

    // synchronize pointers
    syncPointers();

#ifdef BEATMUP_DEBUG
    consistencyTest(fragments);
#endif
}


Sequence* Sequence::copy(dtime fromTime, dtime toTime) const {
    if (fromTime >= toTime)
        return nullptr;		// nothing to copy

    int
        fromFragment = findFragment(fromTime),
        toFragment = findFragment(toTime);

    // whole sequence is apart
    if ((toFragment == VOID_LEFT || toFragment == VOID_RIGHT) && fromFragment == toFragment)
        return nullptr;

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
    dtime dt = fromTime - cumTimes[fromFragment];
    result->fragments.emplace_back(fragments[fromFragment]);
    result->fragments[0].offset += dt;

    // special case: single fragment copied
    if (fromFragment == toFragment) {
        dt = toTime - fromTime;
        result->fragments[0].length = dt;
        result->cumTimes.push_back(dt);
    }
    else {
        dtime cumTime = (result->fragments[0].length -= dt);
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
        dtime l = toTime - cumTimes[toFragment];
        result->fragments.back().length = l;
        result->cumTimes.push_back(cumTime + l);
    }

#ifdef BEATMUP_DEBUG
    consistencyTest(fragments);
#endif
    return result;
}


void Sequence::insert(const Sequence& sequence, dtime time) {
    // check if the time is okay
    if (time < 0 || time > getLength())		// time may be equal to the length
        throw AccessException("Bad insert position", *this);

    // find the fragment (guaranteeing that it is split it in two)
    int index = findFragment(time);
    if (time > cumTimes[index]) {
        dtime dt = time - cumTimes[index];
        splitFragment(index, dt);
        index++;
        cumTimes[index] = cumTimes[index - 1] + dt;
    }

    // insert remaining part of the sequence
    fragments.insert(fragments.begin() + index, sequence.fragments.begin(), sequence.fragments.end());
    cumTimes.resize(fragments.size() + 1);
    dtime cumTime = cumTimes[index];
    for (auto it = fragments.begin() + index; it != fragments.end(); ++it)
        cumTimes[++index] = (cumTime += it->length);

    syncPointers();
#ifdef BEATMUP_DEBUG
    consistencyTest(fragments);
#endif
}


void Sequence::remove(dtime fromTime, dtime toTime) {
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
    dtime dt = fromTime - cumTimes[fromFragment];
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
        dtime dt = fromTime - cumTimes[fromFragment];
        splitFragment(fromFragment, dt);
        dt = toTime - fromTime;
        int index = fromFragment + 1;
        fragments[index].offset += dt;
        fragments[index].length -= dt;
    }

    // update cumulative time index
    cumTimes.resize(fragments.size() + 1);
    int index = fromFragment;
    dtime cumTime = cumTimes[index];
    for (auto it = fragments.begin() + index; it != fragments.end(); ++it)
        cumTimes[++index] = (cumTime += it->length);

    syncPointers();

#ifdef BEATMUP_DEBUG
    consistencyTest(fragments);
#endif
}


void Sequence::split(int time, Sequence* left, Sequence* right) {
    const dtime l = getLength();
    if (time <= 0) {
        left = nullptr;
        right = this;
    }
    else if (time >= l - 1) {
        left = this;
        right = nullptr;
    }
    else {
        left = this;
        right = copy(time, l);
        shrink(0, time);
    }
}



Sequence::Pointer::Pointer(Sequence& sequence, dtime time, bool writing) : writing(writing), watching(false), sequence(sequence) {
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


void Sequence::Pointer::moveTo(dtime time) {
    currentTime = time;
    fragmentIdx = sequence.findFragment(time);
    if (fragmentIdx != VOID_LEFT && fragmentIdx != VOID_RIGHT) {
        if (writing)
            sequence.fragments[fragmentIdx].editData();
        pointer = sequence.fragments[fragmentIdx];
        dtime dt = time - sequence.cumTimes[fragmentIdx];
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

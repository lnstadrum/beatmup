#include "audio_signal_fragment.h"
#include <math.h>

using namespace Beatmup;

/**
	Measures dynamics from samples for a signle channel in a multiplexed stream
	\param startSample		pointer to the first sample the measurement starts from
	\param stopSample		pointer to the last sample where the measurement stops
	\param min				discovered minima value; the passed value is not reset
	\param max				discovered maxima value; the passed value is not reset
*/
template<typename sample> inline void measureMultiplexedChannelDynamics(
	const sample* startSample,
	const sample* stopSample,
	const unsigned char skipSamples,
	sample& min,
	sample& max
) {
	const sample *in = startSample;
	sample vMin = *in, vMax = *in;
	in += skipSamples;
	while (in < stopSample) {
		vMin.x = std::min(vMin.x, in->x);
		vMax.x = std::max(vMax.x, in->x);
		in += skipSamples;
	}
	min.x = std::min(min.x, vMin.x);
	max.x = std::max(max.x, vMax.x);
}


void AudioSignalFragment::DynamicsLookup::disposeTree() {
	if (prev) {
		delete prev;
		prev = NULL;
	}
	if (minmax)
		free(minmax);
	size = 0;
}


void AudioSignalFragment::DynamicsLookup::configureTree(unsigned char channelCount, int levelCount, int fineStepSize, int coarserStepSize) {
	this->channelCount = channelCount;
	if (levelCount == 1) {
		// fine level
		if (prev)
			delete prev;
		stepTime = step = fineStepSize;
		prev = NULL;
	}
	else if (levelCount > 0) {
		if (!prev)
			prev = new DynamicsLookup();
		step = coarserStepSize;
		prev->configureTree(channelCount, levelCount - 1, fineStepSize, coarserStepSize);
		stepTime = step * prev->stepTime;
	}
}


template<typename sample> inline void AudioSignalFragment::DynamicsLookup::updateTree(const sample* data, int sampleCount) {
	// if there is a finer scale ...
	if (prev) {
		// ... send sample data to it recursively
		prev->updateTree(data, sampleCount);
		// update the current level: reallocate table first
		int newSize = ceili(prev->size, step);
		if (newSize != size) {
			minmax = realloc(minmax, newSize * 2 * channelCount * sizeof(sample));
			size = newSize;
		}
		// fill it then
		sample *out = (sample*)minmax;
		const int skip = 2 * channelCount;
		for (int block = 0; block < prev->size; block += step) {
			const int blockSize = std::min(step, prev->size - block);
			// channel multiplexing
			for (int ch = 0; ch < channelCount; ch++) {
				sample *in = (sample*)prev->minmax + block * skip + 2 * ch, *stop = in + blockSize * skip;
				sample vMin = in[0], vMax = in[1];
				in += skip;
				while (in < stop) {
					vMin = std::min(vMin, in[0]);
					vMax = std::max(vMax, in[1]);
					in += skip;
				}
				*(out++) = vMin;
				*(out++) = vMax;
			}
		}
	}
	// update finest scale level: reallocate table first
	else {
		int newSize = ceili(sampleCount, step);
		if (newSize != size) {
			minmax = realloc(minmax, newSize * 2 * channelCount * sizeof(sample));
			size = newSize;
		}
		// fill it then
		sample *out = (sample*)minmax;
		for (int time = 0; time < sampleCount; time += step) {
			const int blockSize = std::min(step, sampleCount - time);
			const sample *start = data + time * channelCount, *stop = start + blockSize * channelCount;
			// channel demultiplexing
			for (int ch = 0; ch < channelCount; ch++) {
				sample min{ sample::MAX_VALUE }, max{ sample::MIN_VALUE };
				measureMultiplexedChannelDynamics(start + ch, stop + ch, channelCount, min, max);
				out[0] = min;
				out[1] = max;
				out += 2;
			}
		}
	}
}

template void AudioSignalFragment::DynamicsLookup::updateTree(const sample8* data, int sampleCount);
template void AudioSignalFragment::DynamicsLookup::updateTree(const sample16* data, int sampleCount);
template void AudioSignalFragment::DynamicsLookup::updateTree(const sample32* data, int sampleCount);
template void AudioSignalFragment::DynamicsLookup::updateTree(const sample32f* data, int sampleCount);


template<typename sample> void AudioSignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample* min, sample* max, const void* data) const {
	int b0, b1;			// bounding block indices
	// if there is a finer level, take the inset and ask for the remaining parts this finer level
	if (prev) {
		b0 = ceili(time0, stepTime);
		b1 = time1 / stepTime;
		dtime
			t0 = b0 * stepTime,
			t1 = b1 * stepTime;
		if (t0 >= t1) {
			// this level is too coarse
			prev->measure<sample>(time0, time1, min, max, data);
			return;
		}

		if (time0 < t0)
			prev->measure<sample>(time0, t0, min, max, data);
		if (t1 < time1)
			prev->measure<sample>(t1, time1, min, max, data);
	}

	// if not, but if there is some sample data, take an inset and measure leftovers using the sample data
	else if (data) {
		b0 = ceili(time0, stepTime);
		b1 = time1 / stepTime;
		// if there are some blocks (more than one)
		if (b0 < b1) {
			dtime
				t0 = b0 * stepTime,
				t1 = b1 * stepTime;
			if (time0 < t0 || t1 < time1) {
				sample *pMin = min, *pMax = max;
				for (int ch = 0; ch < channelCount; ch++, pMin++, pMax++) {
					if (time0 < t0)
						measureMultiplexedChannelDynamics(
							(sample*)data + time0 * channelCount + ch,
							(sample*)data + t0 * channelCount + ch,
							channelCount,
							*pMin, *pMax
						);
					if (t1 < time1)
						measureMultiplexedChannelDynamics(
							(sample*)data + t1 * channelCount + ch,
							(sample*)data + time1 * channelCount + ch,
							channelCount,
							*pMin, *pMax
						);
				}
			}
		}
		// the time borders are both within the same block
		else {
			for (int ch = 0; ch < channelCount; ch++, min++, max++)
				measureMultiplexedChannelDynamics(
					(sample*)data + time0 * channelCount + ch,
					(sample*)data + time1 * channelCount + ch,
					channelCount,
					*min, *max
				);
			return;
		}
	}

	// otherwise take the outset
	else {
		b0 = time0 / stepTime;
		b1 = std::max(b0+1, ceili(time1, stepTime));
	}

	BEATMUP_ASSERT_DEBUG(b1 <= size);

	// scan selected block set
	const int skip = 2 * channelCount;
	for (int chSkip = 0; chSkip < skip; chSkip += 2) {
		sample *ptr = (sample*)minmax + skip*b0 + chSkip, *stop = (sample*)minmax + skip*b1 + chSkip;
		while (ptr < stop) {
			min->x = std::min(ptr[0].x, min->x);
			max->x = std::max(ptr[1].x, max->x);
			ptr += skip;
		}
		min++;
		max++;
	}
}

template void AudioSignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample8* min, sample8* max, const void* data) const;
template void AudioSignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample16* min, sample16* max, const void* data) const;
template void AudioSignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample32* min, sample32* max, const void* data) const;
template void AudioSignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample32f* min, sample32f* max, const void* data) const;


AudioSignalFragment::DynamicsLookup::~DynamicsLookup() {
	disposeTree();
}


AudioSignalFragment::AudioSignalFragment(Environment& env, AudioSampleFormat format, unsigned char channels, int samples) :
env(env), format(format), channelCount(channels)
{
	blockSize = channelCount * AUDIO_SAMPLE_SIZE[format];
	sampleCount = samples;
	data = env.allocateMemory(getSizeBytes());
}


AudioSignalFragment::~AudioSignalFragment() {
	env.freeMemory(data);
}


AudioSignalFragment* AudioSignalFragment::clone() const {
	AudioSignalFragment* copy = new AudioSignalFragment(env, format, channelCount, getSampleCount());
	memcpy(env.acquireMemory(copy->data), env.acquireMemory(data), getSizeBytes());
	env.releaseMemory(data, false);
	env.releaseMemory(copy->data, false);
	copy->plot.fineLevelStep = plot.fineLevelStep;
	copy->plot.coarserLevelStep = plot.coarserLevelStep;
	// do not copy the lookup; it will be recomputed when needed
	copy->plot.lookup.disposeTree();
	return copy;
}


void AudioSignalFragment::zero() {
	void* ptr = env.acquireMemory(data);
	memchr(ptr, 0, getSizeBytes());
	env.releaseMemory(data, false);
}


void AudioSignalFragment::updateDynamicsLookup() {
	plot.lookup.configureTree(
		channelCount,
		ilogb(sampleCount / channelCount / plot.fineLevelStep) / ilogb(plot.coarserLevelStep),
		plot.fineLevelStep,
		plot.coarserLevelStep
	);

	const void* dataPtr = env.acquireMemory(data);
	switch (format) {
	case Int8:
		plot.lookup.updateTree<sample8>((sample8*)dataPtr, sampleCount);
		break;
	case Int16:
		plot.lookup.updateTree<sample16>((sample16*)dataPtr, sampleCount);
		break;
	case Int32:
		plot.lookup.updateTree<sample32>((sample32*)dataPtr, sampleCount);
		break;
	case Float32:
		plot.lookup.updateTree<sample32f>((sample32f*)dataPtr, sampleCount);
		break;
	default:
		Insanity::insanity("Unknown sample format");
	}
	env.releaseMemory(data, false);
}


template<typename sample> void AudioSignalFragment::measureDynamics(int time0, int time1, sample* min, sample* max, AudioSignal::Meter::MeasuringMode mode) {
	BEATMUP_ASSERT_DEBUG(time0 <= time1);
	BEATMUP_ASSERT_DEBUG(AUDIO_SAMPLE_SIZE[format] == sizeof(sample));

	const bool useLookup =
		mode == AudioSignal::Meter::MeasuringMode::approximateUsingLookup ||
		mode == AudioSignal::Meter::MeasuringMode::preciseUsingLookupAndSamples;

	if (useLookup)
		RuntimeError::check(plot.lookup.isReady(), "Dynamics lookup is not ready");

	const void* sampleData = NULL;

	// precise measurement
	if (
		mode == AudioSignal::Meter::MeasuringMode::preciseUsingSamples ||
		mode == AudioSignal::Meter::MeasuringMode::preciseUsingLookupAndSamples
	)
		sampleData = env.acquireMemory(data);

	// use lookup
	if (useLookup)
		plot.lookup.measure(time0, time1, min, max, (sample*)sampleData);

	// don't use lookup
	else {
		BEATMUP_ASSERT_DEBUG(sampleData);
		for (int ch = 0; ch < channelCount; ch++, min++, max++)
			measureMultiplexedChannelDynamics(
				(sample*)sampleData + time0*channelCount + ch,
				(sample*)sampleData + time1*channelCount + ch,
				channelCount,
				*min, *max
			);
	}

	if (sampleData)
		env.releaseMemory(data, false);
}

template void AudioSignalFragment::measureDynamics(int time0, int time1, sample8* min, sample8* max, AudioSignal::Meter::MeasuringMode mode);
template void AudioSignalFragment::measureDynamics(int time0, int time1, sample16* min, sample16* max, AudioSignal::Meter::MeasuringMode mode);
template void AudioSignalFragment::measureDynamics(int time0, int time1, sample32* min, sample32* max, AudioSignal::Meter::MeasuringMode mode);
template void AudioSignalFragment::measureDynamics(int time0, int time1, sample32f* min, sample32f* max, AudioSignal::Meter::MeasuringMode mode);

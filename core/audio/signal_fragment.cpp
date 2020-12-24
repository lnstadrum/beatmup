/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "signal_fragment.h"
#include <math.h>

using namespace Beatmup;
using namespace Audio;

/**
    Measures dynamics from samples for a single channel in a multiplexed stream
    \param startSample	    Pointer to the first sample the measurement starts from
    \param stopSample		Pointer to the last sample where the measurement stops
    \param stride           Step size in samples
    \param min				Discovered minima value; the passed value is not reset
    \param max				Discovered maxima value; the passed value is not reset
*/
template<typename sample> inline void measureMultiplexedChannelDynamics(
    const sample* startSample,
    const sample* stopSample,
    const unsigned char stride,
    sample& min,
    sample& max
) {
    const sample *in = startSample;
    sample vMin = *in, vMax = *in;
    in += stride;
    while (in < stopSample) {
        vMin.x = std::min(vMin.x, in->x);
        vMax.x = std::max(vMax.x, in->x);
        in += stride;
    }
    min.x = std::min(min.x, vMin.x);
    max.x = std::max(max.x, vMax.x);
}


void SignalFragment::DynamicsLookup::disposeTree() {
    if (prev) {
        delete prev;
        prev = nullptr;
    }
    if (minmax)
        free(minmax);
    size = 0;
}


void SignalFragment::DynamicsLookup::configureTree(unsigned char channelCount, int levelCount, int fineStepSize, int coarserStepSize) {
    this->channelCount = channelCount;
    if (levelCount == 1) {
        // fine level
        if (prev)
            delete prev;
        stepTime = step = fineStepSize;
        prev = nullptr;
    }
    else if (levelCount > 0) {
        if (!prev)
            prev = new DynamicsLookup();
        step = coarserStepSize;
        prev->configureTree(channelCount, levelCount - 1, fineStepSize, coarserStepSize);
        stepTime = step * prev->stepTime;
    }
}


template<typename sample> inline void SignalFragment::DynamicsLookup::updateTree(const sample* data, int sampleCount) {
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

template void SignalFragment::DynamicsLookup::updateTree(const sample8* data, int sampleCount);
template void SignalFragment::DynamicsLookup::updateTree(const sample16* data, int sampleCount);
template void SignalFragment::DynamicsLookup::updateTree(const sample32* data, int sampleCount);
template void SignalFragment::DynamicsLookup::updateTree(const sample32f* data, int sampleCount);


template<typename sample> void SignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample* min, sample* max, const void* data) const {
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

template void SignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample8* min, sample8* max, const void* data) const;
template void SignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample16* min, sample16* max, const void* data) const;
template void SignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample32* min, sample32* max, const void* data) const;
template void SignalFragment::DynamicsLookup::measure(dtime time0, dtime time1, sample32f* min, sample32f* max, const void* data) const;


SignalFragment::DynamicsLookup::~DynamicsLookup() {
    disposeTree();
}


SignalFragment::SignalFragment(AudioSampleFormat format, unsigned char channels, int samples) :
    format(format), channelCount(channels)
{
    blockSize = channelCount * AUDIO_SAMPLE_SIZE[format];
    sampleCount = samples;
    data = AlignedMemory(getSizeBytes());
}


SignalFragment* SignalFragment::clone() const {
    SignalFragment* copy = new SignalFragment(format, channelCount, getSampleCount());
    memcpy(copy->data(), data(), getSizeBytes());
    copy->plot.fineLevelStep = plot.fineLevelStep;
    copy->plot.coarserLevelStep = plot.coarserLevelStep;
    // do not copy the lookup; it will be recomputed when needed
    copy->plot.lookup.disposeTree();
    return copy;
}


void SignalFragment::zero() {
    memchr(data(), 0, getSizeBytes());
}


void SignalFragment::updateDynamicsLookup() {
    plot.lookup.configureTree(
        channelCount,
        ilogb(sampleCount / channelCount / plot.fineLevelStep) / ilogb(plot.coarserLevelStep),
        plot.fineLevelStep,
        plot.coarserLevelStep
    );

    switch (format) {
    case Int8:
        plot.lookup.updateTree<sample8>(data.ptr<sample8>(), sampleCount);
        break;
    case Int16:
        plot.lookup.updateTree<sample16>(data.ptr<sample16>(), sampleCount);
        break;
    case Int32:
        plot.lookup.updateTree<sample32>(data.ptr<sample32>(), sampleCount);
        break;
    case Float32:
        plot.lookup.updateTree<sample32f>(data.ptr<sample32f>(), sampleCount);
        break;
    default:
        Insanity::insanity("Unknown sample format");
    }
}


template<typename sample> void SignalFragment::measureDynamics(int time0, int time1, sample* min, sample* max, Signal::Meter::MeasuringMode mode) {
    BEATMUP_ASSERT_DEBUG(time0 <= time1);
    BEATMUP_ASSERT_DEBUG(AUDIO_SAMPLE_SIZE[format] == sizeof(sample));

    const bool useLookup =
        mode == Signal::Meter::MeasuringMode::approximateUsingLookup ||
        mode == Signal::Meter::MeasuringMode::preciseUsingLookupAndSamples;

    if (useLookup)
        RuntimeError::check(plot.lookup.isReady(), "Dynamics lookup is not ready");

    const void* sampleData = nullptr;

    // precise measurement
    if (
        mode == Signal::Meter::MeasuringMode::preciseUsingSamples ||
        mode == Signal::Meter::MeasuringMode::preciseUsingLookupAndSamples
    ) {
        sampleData = data();
    }

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
}

template void SignalFragment::measureDynamics(int time0, int time1, sample8* min, sample8* max, Signal::Meter::MeasuringMode mode);
template void SignalFragment::measureDynamics(int time0, int time1, sample16* min, sample16* max, Signal::Meter::MeasuringMode mode);
template void SignalFragment::measureDynamics(int time0, int time1, sample32* min, sample32* max, Signal::Meter::MeasuringMode mode);
template void SignalFragment::measureDynamics(int time0, int time1, sample32f* min, sample32f* max, Signal::Meter::MeasuringMode mode);

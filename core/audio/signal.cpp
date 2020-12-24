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

#include "signal.h"
#include "signal_fragment.h"
#include "processing.h"
#include "wav_utilities.h"
#include "../utils/input_stream.h"
#include "../exception.h"
#include <algorithm>

using namespace Beatmup;
using namespace Audio;


Signal* Signal::createEmpty() const {
    return new Signal(ctx, format, sampleRate, channelCount, (float)defaultFragmentSize / sampleRate);
}


Signal::Signal(Context& context, AudioSampleFormat format, int sampleRate, unsigned char channels, float defaultFragmentLenSec) :
    ctx(context),
    format(format),
    defaultFragmentSize(floorf_fast(defaultFragmentLenSec * sampleRate)),
    sampleRate(sampleRate),
    channelCount(channels)
{
    BEATMUP_ASSERT_DEBUG(defaultFragmentSize > 0);
}


void Signal::insert(const Signal& signal, dtime time) {
    if (signal.channelCount != channelCount || signal.format != format)
        throw Signal::IncompatibleFormat(signal, *this);
    Sequence::insert(signal, time);
}


void Signal::reserve(dtime length) {
    int curDur;
    while ((curDur = getDuration()) < length) {
        SignalFragment* newbie = new SignalFragment(format, channelCount, defaultFragmentSize);
        newbie->zero();
        concatenate(*newbie, 0, std::min(defaultFragmentSize, length - curDur));
    }
}


Signal* Signal::loadWAV(Context& ctx, const char* filename) {
    // open file
    FileInputStream stream(filename);
    if (!stream.isOpen())
        throw IOError(filename, "Unable to open for reading");
    return Signal::loadWAV(ctx, stream);
}

Signal *Signal::loadWAV(Context &ctx, InputStream& stream) {
    // read header
    WAV::Header header;
    stream(&header, sizeof(header));
    WAV::InvalidWavFile::check(header);

    // get sample format
    AudioSampleFormat format;
    switch (header.bitsPerSample) {
        case 8:
            format = Int8;
            break;
        case 16:
            format = Int16;
            break;
        case 32:
            format = Int32;
            break;
        default:
            throw WAV::InvalidWavFile("Incorrect WAV file: unsupported BPS");
    }

    // check number of channels (insanity check, basically)
    if (header.numChannels <= 0 || header.numChannels > 255)
        throw WAV::InvalidWavFile("Incorrect WAV file: unsupported channel count");

    // create signal
    Signal* signal = new Signal(ctx, format, header.sampleRate, header.numChannels);
    dtime totalTime = header.dataSizeBytes / (header.numChannels * header.bitsPerSample / 8);
    signal->reserve(totalTime);

    // read data using pointer
    Writer pointer(*signal, 0);
    while (pointer.hasData() && !stream.eof()) {
        void* data;
        dtime length = pointer.acquireBuffer(data);
        stream(data, length * header.numChannels * AUDIO_SAMPLE_SIZE[format]);
        pointer.releaseBuffer();
        pointer.jump(length);
    }

    return signal;
}


void Signal::saveWAV(const char* filename) {
    // open file
    std::ofstream file(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    if (!file.is_open())
        throw IOError(filename, "Unable to open for writing");

    // init and write header
    unsigned char sampleSize = AUDIO_SAMPLE_SIZE[format];
    WAV::Header header;
    header.set(sampleRate, sampleSize * 8, channelCount, getDuration() * channelCount * sampleSize);
    file.write((char*)&header, sizeof(header));

    // write out
    Reader pointer(*this, 0);
    while (pointer.hasData() && !file.eof()) {
        if (file.fail())
            throw IOError(filename, "Failed while writing");
        const void* data;
        dtime length = pointer.acquireBuffer(data);
        file.write((const char*)data, length * channelCount * sampleSize);
        pointer.releaseBuffer();
        pointer.jump(length);
    }
}


Signal::Pointer::Pointer(Signal& signal, dtime time, bool writing): Sequence::Pointer(signal, time, writing)
{}


void Signal::Pointer::releaseBuffer() {
}


unsigned char Signal::Pointer::getChannelCount() const {
    return ((SignalFragment*)pointer.fragment)->getChannelCount();
}


Signal::Reader::Reader(Signal& signal, dtime time) : Pointer(signal, time, false) {};


dtime Signal::Reader::acquireBuffer(const void* &data) {
    if (pointer.isNull()) {
        data = nullptr;
        return 0;
    }
    SignalFragment* fragment = (SignalFragment*)pointer.fragment;
    data = fragment->getData() + pointer.offset * fragment->getBlockSize();
    return pointer.length;
}


Signal::Writer::Writer(Signal& signal, dtime time) : Pointer(signal, time, true) {};

dtime Signal::Writer::acquireBuffer(void* &data) {
    if (pointer.isNull()) {
        data = nullptr;
        return 0;
    }
    SignalFragment* fragment = (SignalFragment*)pointer.fragment;
    data = fragment->getData() + pointer.offset * fragment->getBlockSize();
    return pointer.length;
}


void Signal::Meter::prepare(Signal& signal, int skipOnStart, int numSteps) {
    Meter me(signal, 0, MeasuringMode::approximateUsingLookup);
    for (int skip = 0; skip < skipOnStart; skip++)
        me.step();
    while (me.hasData()) {
        SignalFragment* fragment = (SignalFragment*)me.pointer.fragment;
        if (fragment && !fragment->isDynamicsLookupAvailable())
            fragment->updateDynamicsLookup();
        for (int skip = 0; skip < numSteps; skip++)
            me.step();
    }
}


Signal::Meter::Meter(Signal& signal, dtime time, MeasuringMode mode):
    Pointer(signal, time, false), mode(mode)
{}


void Signal::Meter::prepareSignal(Signal& signal, bool runTask) {
    if (runTask) {

        class DynamicsLookupPreparation : public AbstractTask {
        public:
            Signal& signal;
            bool process(TaskThread& thread) {
                // interleaving fragments among threads
                Meter::prepare(signal, thread.currentThread(), thread.numThreads());
                return true;
            }

            TaskDeviceRequirement getExecutionMode() const { return AbstractTask::TaskDeviceRequirement::CPU_ONLY; }
            ThreadIndex getMaxThreads() const { return MAX_THREAD_INDEX; }
            DynamicsLookupPreparation(Signal& signal) : signal(signal) {}
        };

        DynamicsLookupPreparation task(signal);
        signal.getContext().performTask(task);
    }
    else
        Meter::prepare(signal);
}


template<typename sample> void Signal::Meter::measureInFragments(dtime len, int resolution, sample* min, sample* max) {
    const int channelCount = ((SignalFragment*)pointer.fragment)->getChannelCount();
    const dtime startTime = getTime();
    dtime t1 = startTime;
    sample *pMin = min, *pMax = max;

    // for all bins
    for (int bin = 1; bin <= resolution; bin++) {
        // determining bin bounds
        dtime t0 = t1;
        t1 = startTime + (long long)len * bin / resolution;

        // determining start time within fragment
        dtime fragStart = t0 - getTime() + pointer.offset;

        // initializing min and max values
        for (int ch = 0; ch < channelCount; ch++) {
            pMin[ch].x = sample::MAX_VALUE;
            pMax[ch].x = sample::MIN_VALUE;
        }

        // scanning fragments
        while (getTime() + pointer.length < t1) {
            ((SignalFragment*)pointer.fragment)->measureDynamics(fragStart, pointer.offset + pointer.length, pMin, pMax, mode);
            step();
            fragStart = pointer.offset;
        }
        ((SignalFragment*)pointer.fragment)->measureDynamics(fragStart, pointer.offset + t1 - getTime(), pMin, pMax, mode);

        // if no values defined
        for (int ch = 0; ch < channelCount; ch++)
            if (pMax[ch] < pMin[ch])
                pMin[ch].x = pMax[ch].x = 0;
        pMin += channelCount;
        pMax += channelCount;
    }
}

template void Signal::Meter::measureInFragments(dtime len, int resolution, sample8* min, sample8* max);
template void Signal::Meter::measureInFragments(dtime len, int resolution, sample16* min, sample16* max);
template void Signal::Meter::measureInFragments(dtime len, int resolution, sample32* min, sample32* max);
template void Signal::Meter::measureInFragments(dtime len, int resolution, sample32f* min, sample32f* max);


namespace Beatmup {		// why? Because of a bug in gcc

    template<> void Signal::Meter::measure(dtime len, int resolution, sample16 min[], sample16 max[]) {
        const AudioSampleFormat fmt = ((SignalFragment*)pointer.fragment)->getAudioFormat();
        if (fmt == Int16) {
            measureInFragments(len, resolution, (sample16*)min, (sample16*)max);
            return;
        }
        // converting min/max types
        const int size = resolution * ((SignalFragment*)pointer.fragment)->getChannelCount();
        switch (fmt) {
        case Int8: {
            sample8 *tmin = new sample8[size], *tmax = new sample8[size];
            measureInFragments(len, resolution, tmin, tmax);
            convertSamples<sample8, sample16>(tmin, min, resolution);
            convertSamples<sample8, sample16>(tmax, max, resolution);
            delete[] tmin;
            delete[] tmax;
            break;
        }
        case Int32: {
            sample32 *tmin = new sample32[size], *tmax = new sample32[size];
            measureInFragments(len, resolution, tmin, tmax);
            convertSamples<sample32, sample16>(tmin, min, resolution);
            convertSamples<sample32, sample16>(tmax, max, resolution);
            delete[] tmin;
            delete[] tmax;
            break;
        }
        case Float32: {
            sample32f *tmin = new sample32f[size], *tmax = new sample32f[size];
            measureInFragments(len, resolution, tmin, tmax);
            convertSamples<sample32f, sample16>(tmin, min, resolution);
            convertSamples<sample32f, sample16>(tmax, max, resolution);
            delete[] tmin;
            delete[] tmax;
            break;
        }
        default:
            Insanity::insanity("Unknown audio format");
        }
    }
}


namespace Kernels {
    template<typename in_t, typename out_t> class RenderAudio {
    public:
        /**
            Renders a continuous audio signal from Signal::Reader
        */
        static void process(const in_t* whatever, out_t* output, Signal::Reader& ptr, dtime length, const int inCh, const int outCh) {
            static const out_t _0{ 0 };
            const int numc = (int)std::min(inCh, outCh);
            while (length > 0 && ptr.hasData()) {
                const void* data;
                int chunk = ptr.acquireBuffer(data);
                if (chunk > length)
                    chunk = length;

                const in_t* input = (const in_t*)data;
                for (int t = 0; t < chunk; ++t, input += inCh, output += outCh) {
                    int c = 0;
                    for (; c < numc; ++c)
                        output[c] = input[c];
                    for (; c < outCh; ++c)
                        output[c] = _0;
                }

                ptr.releaseBuffer();
                ptr.jump(chunk);
                length -= chunk;
            }

            memset(output, 0, length * outCh);
        }
    };
}


Signal::Source::Source(Signal& signal): signal(&signal) {}


void Signal::Source::prepare(
        const dtime sampleRate,
        const AudioSampleFormat sampleFormat,
        const unsigned char numChannels,
        const dtime maxBufferLen
) {
    this->numChannels = numChannels;
    this->sampleFormat = sampleFormat;
}


void Signal::Source::setClock(dtime time) {
    this->time = time;
}


void Signal::Source::render(
    TaskThread& thread,
    sample8* buffer,
    const dtime bufferLength
) {
    Reader ptr(*signal, time);
    Processing::pipeline<Kernels::RenderAudio>(signal->getSampleFormat(), sampleFormat, nullptr, buffer, ptr, bufferLength, signal->getChannelCount(), numChannels);
    time += bufferLength;
}


Signal::IncompatibleFormat::IncompatibleFormat(const Signal& source, const Signal& dest):
    Exception(
            "Incompatible format: %s @ %d channels vs <-> %s @ %d channels",
            AUDIO_FORMAT_NAME[source.getSampleFormat()], source.getChannelCount(),
            AUDIO_FORMAT_NAME[dest.getSampleFormat()], dest.getChannelCount()
    ),
    sourceFormat(source.getSampleFormat()), destFormat(dest.getSampleFormat()), sourceChannelCount(source.getChannelCount()), destChannelCount(dest.getChannelCount())
{}

#include "source.h"
#include "../exception.h"
#include <cmath>
#include <core/debug.h>

using namespace Beatmup;
using namespace Audio;


void Source::setClock(dtime time) {
    // nothing to do by default
}


template <typename sample> inline void fillSin(
        psample* buffer,
        dtime time,
        dtime length,
        float volume,
        float frequency,
        float phase,
        dtime sampleRate,
        unsigned char numChannels,
        int scale)
{
    dtime stop = time + length;
    sample* s = (sample*)buffer;
    float f = 2 * pi * frequency / sampleRate;
    sample v;
    for (dtime t = time; t < stop; ++t) {
        v.x = scale * clamp(-1.0f, volume * std::sin(t * f + phase), 1.0f);
        for (unsigned char c = 0; c < numChannels; c++)
            (s++)->x = v.x;
    }
}


void HarmonicSource::prepare(
        const dtime sampleRate,
        const AudioSampleFormat sampleFormat,
        const unsigned char numChannels,
        const dtime maxBufferLen)
{
    this->sampleFormat = sampleFormat;
    this->sampleRate = sampleRate;
    this->numChannels = numChannels;
}


void HarmonicSource::render(TaskThread& thread, psample* buffer, const dtime bufferLength) {
    BEATMUP_DEBUG_I("FILLING BUFFER OF %d SAMPLES at time %d", bufferLength, time);
    switch (sampleFormat) {
        case Int8:
            fillSin<sample8>(buffer, time, bufferLength, amplitude, frequency, phase, sampleRate, numChannels, 127);
            break;

        case Int16:
            fillSin<sample16>(buffer, time, bufferLength, amplitude, frequency, phase, sampleRate, numChannels, 0x7FFF);
            break;

        case Int32:
            fillSin<sample32>(buffer, time, bufferLength, amplitude, frequency, phase, sampleRate, numChannels, 0x7FFFFFFF);
            break;

        case Float32:
            fillSin<sample32f>(buffer, time, bufferLength, amplitude, frequency, phase, sampleRate, numChannels, 1);
            break;

        default:
            Insanity::insanity("Unsupported sample format");
    }
    time += bufferLength;
}


void HarmonicSource::setClock(dtime time) {
    this->time = time;
}

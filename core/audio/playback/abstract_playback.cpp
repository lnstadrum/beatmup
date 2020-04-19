#include "../../debug.h"
#include "abstract_playback.h"

using namespace Beatmup;
using namespace Audio;

PlaybackException::PlaybackException(const char *message, int resultCode, const AbstractPlayback::Mode& mode):
    Exception("%s\nResult code: %d\nSampling mode: %d\nSample format: %d\nChannels: %d\nBuffers: %d of %d samples\n",
              message, resultCode, mode.sampleRate, mode.sampleFormat, mode.numChannels, mode.numBuffers, mode.bufferLength)
{}

PlaybackException::PlaybackException(const char *message, int resultCode):
    Exception("%s\nResult code: %d", message, resultCode)
{}



AbstractPlayback::Mode::Mode(dtime sampleRate, AudioSampleFormat format, int numChannels,
                             dtime bufferLength, int numBuffers):
    sampleRate(sampleRate),
    sampleFormat(format),
    numChannels((unsigned char) numChannels),
    bufferLength(bufferLength),
    numBuffers((unsigned char) numBuffers)
{
    // check that all the mode parameters are in their valid ranges
    if (numBuffers < 2)
        Insanity::insanity("Bad playback mode: less that 2 buffers");
    if (numChannels < 1)
        Insanity::insanity("Bad playback mode: no channels");
    if (bufferLength < 1)
        Insanity::insanity("Bad playback mode: non-positive buffer length");
    switch (format) {
        case Int8:
        case Int16:
        case Int32:
        case Float32:
            break;
        default:
            Insanity::insanity("Bad playback mode: invalid sample format");
    }
        // we accept zero/negative sample rate which signifies "not set"
}

AbstractPlayback::Mode::Mode(dtime sampleRate, AudioSampleFormat format, int numChannels,
                             dtime bufferLength):
    Mode(sampleRate, format, numChannels, bufferLength, 2)
{}

AbstractPlayback::Mode::Mode(): Mode(0, AudioSampleFormat::Int16, 1, 1024, 2) {}



AbstractPlayback::AbstractPlayback(): source(nullptr), clock(0) {}

void AbstractPlayback::initialize(AbstractPlayback::Mode mode) {
    this->mode = mode;
    clock = 0;
}


ThreadIndex AbstractPlayback::maxAllowedThreads() const {
    return source ? source->maxAllowedThreads() : 1;
}

void AbstractPlayback::beforeProcessing(ThreadIndex threadCount, GraphicPipeline *gpu) {
    if (source) {
        source->prepare(mode.sampleRate, mode.sampleFormat, mode.numChannels, mode.bufferLength);
        source->setClock(clock);
    }
}

void AbstractPlayback::advanceTime() {
    clock += mode.bufferLength;
}

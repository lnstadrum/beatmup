#include <core/debug.h>
#include "realtime_playback.h"

using namespace Beatmup;
using namespace Audio;


BasicRealtimePlayback::BasicRealtimePlayback(): buffers(nullptr)
{}


BasicRealtimePlayback::~BasicRealtimePlayback() {
    freeBuffers();
}


void BasicRealtimePlayback::freeBuffers() {
    if (buffers == nullptr)
        return;
    for (int i = 0; i < numBuffers; i++)
        freeBuffer(buffers[i]);
    delete[] buffers;
    buffers = nullptr;
}


void BasicRealtimePlayback::prepareBuffers(const AbstractPlayback::Mode& mode) {
    freeBuffers();
    numBuffers = mode.numBuffers;
    buffers = new psample*[numBuffers];
    for (int i = 0; i < numBuffers; i++)
        buffers[i] = createBuffer(mode);
}


psample* BasicRealtimePlayback::createBuffer(const AbstractPlayback::Mode& mode) const {
    psample* buffa = (psample*) malloc(bufferSize);
    std::memset(buffa, 0, bufferSize);
    return buffa;
}


void BasicRealtimePlayback::freeBuffer(psample* buffa) const {
    free(buffa);
}


bool BasicRealtimePlayback::sendNextBuffer() {
    std::lock_guard<std::mutex> lock(queueAccess);
    if (sendIndex >= fillIndex)
        return false;
    BEATMUP_DEBUG_I("Enqueueing buffer %d", sendIndex);
    enqueueBuffer(buffers[sendIndex % numBuffers], sendIndex);
    sendIndex++;
    underrun = false;
    return true;
}


psample *BasicRealtimePlayback::getCurrentBuffer() const {
//    DEBUG_I("getCurrentBuffer: fillIndex = %d, playIndex = %d, underrun = %d", fillIndex, playIndex, (int)underrun);
    if (!underrun && (fillIndex - playIndex) % numBuffers == 0)
        return nullptr;
    return buffers[fillIndex % numBuffers];
}


bool BasicRealtimePlayback::goToNextBuffer() {
    if (!underrun && (fillIndex - playIndex) % numBuffers == 0)
        return false;
    fillIndex++;
    BEATMUP_DEBUG_I("goToNextBuffer: fillIndex = %d", fillIndex);
    advanceTime();
    // if underrun, to continue playing we have to send something
    if (underrun)
        sendNextBuffer();
    return true;
}


void BasicRealtimePlayback::initialize(Mode mode) {
    AbstractPlayback::initialize(mode);

    // set default buffer size
    bufferSize = (msize) (mode.bufferLength * mode.numChannels * AUDIO_SAMPLE_SIZE[mode.sampleFormat]);

    // prepare buffers
    prepareBuffers(mode);

    // setting up control variables
    playIndex = sendIndex = fillIndex = 0;
    underrun = true;
}


bool BasicRealtimePlayback::process(TaskThread &thread) {
    if (source)
        while (psample *buffa = getCurrentBuffer()) {
            source->render(thread, buffa, mode.bufferLength);
            if (thread.isManaging())
                goToNextBuffer();
            thread.synchronize();
        }
    return source != nullptr;
}

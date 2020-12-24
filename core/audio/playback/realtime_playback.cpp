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

#include "../../debug.h"
#include "realtime_playback.h"

using namespace Beatmup;
using namespace Audio;


BasicRealtimePlayback::BasicRealtimePlayback(OutputMode mode): outputMode(mode), buffers(nullptr)
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
    buffers = new sample8*[numBuffers];
    for (int i = 0; i < numBuffers; i++)
        buffers[i] = createBuffer(mode);
}


sample8* BasicRealtimePlayback::createBuffer(const AbstractPlayback::Mode& mode) const {
    sample8* buffer = (sample8*) malloc(bufferSize);
    std::memset(buffer, 0, bufferSize);
    return buffer;
}


void BasicRealtimePlayback::freeBuffer(sample8* buffer) const {
    free(buffer);
}


void BasicRealtimePlayback::initialize(Mode mode) {
    AbstractPlayback::initialize(mode);

    // set default buffer size
    bufferSize = (msize) (mode.bufferLength * mode.numChannels * AUDIO_SAMPLE_SIZE[mode.sampleFormat]);

    // prepare buffers
    prepareBuffers(mode);
}


void BasicRealtimePlayback::start() {
    // setting up control variables
    playIndex = sendIndex = fillIndex = 0;
    playingBufferOffset = 0;
    skipFrames = mode.getLatency();
}


bool BasicRealtimePlayback::process(TaskThread &thread) {
    if (!source)
        return false;

    int myFillIndex = 0;

    while (!thread.isTaskAborted()) {
        source->render(thread, buffers[myFillIndex % numBuffers], mode.bufferLength);
        myFillIndex++;
        if (thread.isManaging()) {
            fillIndex = myFillIndex;
            advanceTime();
            // if pushing output mode, then push
            if (outputMode == OutputMode::PUSH && fillIndex - sendIndex > 0) {
                pushBuffer(buffers[sendIndex % numBuffers], sendIndex);
                sendIndex++;
            }
            // wait till some buffers are available
            while (fillIndex - playIndex >= numBuffers)
                std::this_thread::yield();
        }
        thread.synchronize();
    }

    return true;
}


void BasicRealtimePlayback::pullBuffer(sample8 *buffer, dtime numFrames) {
    const dtime frameSize = AUDIO_SAMPLE_SIZE[mode.sampleFormat] * mode.numChannels;

    // skipping frames first
    if (skipFrames > 0) {
        if (numFrames < skipFrames) {
            skipFrames -= numFrames;
            return;
        }
        else {
            buffer += skipFrames * frameSize;
            numFrames -= skipFrames;
            skipFrames = 0;
        }
    }

    // copying data from internal buffers until the requested number of frames is reached or underrun occurs
    const dtime playingBufferSize = mode.bufferLength;
    while (numFrames > 0 && playIndex - fillIndex < 0) {
        const dtime
            chunk = std::min(numFrames, playingBufferSize - playingBufferOffset),
            numBytes = chunk * frameSize;
        std::memcpy(buffer, buffers[playIndex % numBuffers] + playingBufferOffset * frameSize, (size_t) numBytes);
        numFrames -= chunk;
        playingBufferOffset += chunk;
        buffer += numBytes;
        // switching to the next buffer if it is the moment
        if (playingBufferOffset >= playingBufferSize) {
            playIndex++;
            playingBufferOffset = 0;
        }
    }

    // check for underrun
    if (playIndex - fillIndex >= 0 && numFrames > 0) {
        skipFrames = mode.getLatency();
        BEATMUP_DEBUG_I("UNDERRUN");
    }
}

/*
    Playback implementation via OpenSL ES
 */
#pragma once

#include <core/audio/playback/realtime_playback.h>

namespace Beatmup {
    namespace Android {

        class SLESPlayback : public Beatmup::Audio::BasicRealtimePlayback {
        protected:
            class SLESBackend;

            SLESBackend *backend;

            void enqueueBuffer(psample *buffa, int bufferIndex);

        public:
            SLESPlayback();

            ~SLESPlayback();

            void initialize(Mode mode);

            void start();

            void stop();
        };

    }
}
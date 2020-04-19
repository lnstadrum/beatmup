/*
    Playback implementation via OpenSL ES
 */
#pragma once

#include "../realtime_playback.h"

namespace Beatmup {
    namespace Audio {
        namespace Android {

            class SLESPlayback : public Beatmup::Audio::BasicRealtimePlayback {
            protected:
                class SLESBackend;

                SLESBackend *backend;

                void pushBuffer(sample8 *buffer, int bufferIndex);

            public:
                SLESPlayback();

                ~SLESPlayback();

                void initialize(Mode mode);

                void start();

                void stop();
            };

        }
    }
}

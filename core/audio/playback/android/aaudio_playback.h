/*
    Playback implementation via AAudio
*/
#pragma once

#include "../realtime_playback.h"


namespace Beatmup {
    namespace Audio {
        namespace Android {
            class AAudioPlayback : public Beatmup::Audio::BasicRealtimePlayback {
            protected:
                class Backend;
                Backend *backend;

            public:
                AAudioPlayback();
                ~AAudioPlayback();

                void initialize(Mode mode);
                void start();
                void stop();
            };
        }
    }
}

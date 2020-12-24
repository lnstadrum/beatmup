/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#pragma once

#include "../realtime_playback.h"


namespace Beatmup {
    namespace Audio {
        /**
            Android-specific audio API
        */
        namespace Android {
            /**
                AAudio Android playback
            */
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

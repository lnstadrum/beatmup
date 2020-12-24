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

#pragma once
#include "../parallelism.h"
#include "../context.h"

namespace Beatmup {

    /*
        A task to switch display used by GPU.
    */
    class DisplaySwitch : public AbstractTask {
    private:
        void* switchingData;
        bool gpuIsOk;
        bool processOnGPU(GraphicPipeline& gpu, TaskThread&);
        bool process(TaskThread& thread);
        TaskDeviceRequirement getUsedDevices() const;
        DisplaySwitch();
    public:
        static bool run(Context& ctx, void* switchingData = nullptr);
    };
}
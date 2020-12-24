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

#include "display_switch.h"
#include "../gpu/pipeline.h"

using namespace Beatmup;

bool DisplaySwitch::processOnGPU(GraphicPipeline& gpu, TaskThread&) {
    gpuIsOk = true;
#ifdef BEATMUP_PLATFORM_ANDROID
    gpu.switchDisplay(switchingData);
#endif
    return true;
}


bool DisplaySwitch::process(TaskThread& thread) {
    gpuIsOk = false;
    return true;
}


AbstractTask::TaskDeviceRequirement DisplaySwitch::getUsedDevices() const {
    return TaskDeviceRequirement::GPU_OR_CPU;
}


DisplaySwitch::DisplaySwitch() : switchingData(nullptr), gpuIsOk(false) {}


bool DisplaySwitch::run(Context& ctx, void* switchingData) {
    DisplaySwitch task;
    task.switchingData = switchingData;
    ctx.performTask(task);
    return task.gpuIsOk;
}

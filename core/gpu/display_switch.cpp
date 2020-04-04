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


AbstractTask::ExecutionTarget DisplaySwitch::getExecutionTarget() const {
    return ExecutionTarget::useGPUIfAvailable;
}


DisplaySwitch::DisplaySwitch() : switchingData(nullptr), gpuIsOk(false) {}


bool DisplaySwitch::run(Context& ctx, void* switchingData) {
    DisplaySwitch task;
    task.switchingData = switchingData;
    ctx.performTask(task);
    return task.gpuIsOk;
}

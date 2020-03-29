#include "gpu_task.h"
#include "../exception.h"

using namespace Beatmup;

AbstractTask::ExecutionTarget GpuTask::getExecutionTarget() const {
    return ExecutionTarget::useGPU;
}

ThreadIndex GpuTask::maxAllowedThreads() const {
    return 1;
}

bool GpuTask::process(TaskThread &thread) {
    Insanity::insanity("A GPU requiring task is being run with no GPU support provided.");
    return false;
}
#include "gpu_task.h"
#include "../exception.h"

using namespace Beatmup;

AbstractTask::ExecutionTarget GPUTask::getExecutionTarget() const {
    return ExecutionTarget::useGPU;
}

ThreadIndex GPUTask::maxAllowedThreads() const {
    return 1;
}

bool GPUTask::process(TaskThread &thread) {
    Insanity::insanity("A GPU requiring task is run on CPU.");
    return false;
}

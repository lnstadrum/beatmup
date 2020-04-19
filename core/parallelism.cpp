#include "parallelism.h"
#include "exception.h"

using namespace Beatmup;

void AbstractTask::beforeProcessing(ThreadIndex, GraphicPipeline*) {
    // nothing to do by default
}


void AbstractTask::afterProcessing(ThreadIndex, GraphicPipeline*, bool) {
    // nothing to do by default
}


bool AbstractTask::processOnGPU(GraphicPipeline&, TaskThread&) {
    // nothing to do by default
    return true;
}


ThreadIndex AbstractTask::maxAllowedThreads() const {
    return 1;
}


ThreadIndex AbstractTask::validThreadCount(int N) {
    if (N < 1)
        return 1;
    if (N > MAX_THREAD_INDEX)
        return MAX_THREAD_INDEX;
    return N;
}


AbstractTask::ExecutionTarget AbstractTask::getExecutionTarget() const {
    return ExecutionTarget::doNotUseGPU;
}

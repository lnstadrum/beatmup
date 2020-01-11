/*
    A blank class for a task executed on GPU only
*/

#pragma once
#include "../parallelism.h"
#include "../context.h"

namespace Beatmup {
    /**
        Template of a task using GPU
    */
    class GpuTask : public AbstractTask {
    private:
        ExecutionTarget getExecutionTarget() const;
        ThreadIndex maxAllowedThreads() const;
        bool process(TaskThread& thread);
    };
}
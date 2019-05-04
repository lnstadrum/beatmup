/*
    A blank class for a task executed on GPU only
*/

#pragma once
#include "../parallelism.h"

namespace Beatmup {
    class GPUTask : public AbstractTask {
    private:
        ExecutionTarget getExecutionTarget() const;
        ThreadIndex maxAllowedThreads() const;
        bool process(TaskThread& thread);
    };
}
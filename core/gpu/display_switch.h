/*
    A task to switch GPU display
*/

#pragma once
#include "../parallelism.h"
#include "../context.h"

namespace Beatmup {
    class DisplaySwitch : public AbstractTask {
    private:
        void* switchingData;
        bool gpuIsOk;
        bool processOnGPU(GraphicPipeline& gpu, TaskThread&);
        bool process(TaskThread& thread);
        ExecutionTarget getExecutionTarget() const;
        DisplaySwitch();
    public:
        static bool run(Context& ctx, void* switchingData = NULL);
    };
}
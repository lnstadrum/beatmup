/*
    Swapping pixel data from GPU to CPU and vice versa
*/
#pragma once

#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"

namespace Beatmup {
    class Swapper : public GpuTask {
    private:
        AbstractBitmap* bitmap;
        bool fromGpuToCpu;
        bool processOnGPU(GraphicPipeline& gpu, TaskThread&);

    public:
        Swapper(bool fromGpuToCpu);
        void setBitmap(AbstractBitmap&);

        /**
            Copies bitmap from GPU memory to RAM
        */
        static void pullPixels(AbstractBitmap& bitmap);

        /**
            Copies bitmap from RAM to GPU memory
        */
        static void pushPixels(AbstractBitmap& bitmap);
    };
}
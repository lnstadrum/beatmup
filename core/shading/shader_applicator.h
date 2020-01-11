/*
    Lightweight task applying an image shader to a bitmap
*/
#pragma once
#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"
#include "../geometry.h"
#include "image_shader.h"

namespace Beatmup {

    /**
        A task applying an image shader to a bitmap
    */
    class ShaderApplicator : public GpuTask {
    private:
        ImageShader* shader;
        AbstractBitmap *input, *output;
        AffineMapping mapping;

        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, bool aborted);

    public:
        ShaderApplicator();
        void setInputBitmap(AbstractBitmap* bitmap);
        void setOutputBitmap(AbstractBitmap* bitmap);
        void setShader(ImageShader* shader);

        AbstractBitmap* getInputBitmap() const { return input; }
        AbstractBitmap* getOutputBitmap() const { return output; }
        ImageShader* getShader() const { return shader; }
    };
}
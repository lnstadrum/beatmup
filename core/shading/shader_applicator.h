/*
    Lightweight task applying an image shader to a bitmap
*/
#pragma once
#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"
#include "../geometry.h"
#include "image_shader.h"
#include "../utils/bitmap_locker.h"
#include <map>

namespace Beatmup {

    /**
        A task applying an image shader to a bitmap
    */
    class ShaderApplicator : public GpuTask {
    private:
        std::map<std::string, AbstractBitmap*> samplers;
        BitmapLocker locker;
        ImageShader* shader;
        AbstractBitmap *mainInput, *output;
        AffineMapping mapping;

        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);

    public:
        ShaderApplicator();

        /**
            Connects a bitmap to a shader uniform variable.
            The bitmap connected to ImageShader::INPUT_IMAGE_ID is used to resolve the sampler type (ImageShader::INPUT_IMAGE_DECL_TYPE).
        */
        void addSampler(AbstractBitmap* bitmap, const std::string uniformName = ImageShader::INPUT_IMAGE_ID);

        bool removeSampler(const std::string uniformName);
        void clearSamplers();

        void setOutputBitmap(AbstractBitmap* bitmap);
        void setShader(ImageShader* shader);

        AbstractBitmap* getOutputBitmap() const { return output; }
        ImageShader* getShader()          const { return shader; }
        const size_t getSamplersCount()   const { return samplers.size(); }
    };
}

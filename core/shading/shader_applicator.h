/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"
#include "../geometry.h"
#include "image_shader.h"
#include <map>

namespace Beatmup {

    /**
        A task applying an image shader to a bitmap
    */
    class ShaderApplicator : public GpuTask, private BitmapContentLock {
    private:
        std::map<std::string, AbstractBitmap*> samplers;
        ImageShader* shader;
        AbstractBitmap *mainInput, *output;
        AffineMapping mapping;

        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);

    public:
        ShaderApplicator();

        /**
            Connects a bitmap to a shader uniform variable.
            The bitmap connected to ImageShader::INPUT_IMAGE_ID is used to resolve the sampler type (ImageShader::INPUT_IMAGE_DECL_TYPE).
        */
        void addSampler(AbstractBitmap* bitmap, const std::string uniformName = ImageShader::INPUT_IMAGE_ID);

        /**
            Removes a sampler with a uniform variable name.
            \param[in] uniformName      The uniform variable
            \return true if a sampler associated to the given variable existed and was removed, false otherwise.
        */
        bool removeSampler(const std::string uniformName);

        /**
            Clears all connections of bitmaps to samplers
        */
        void clearSamplers();

        void setOutputBitmap(AbstractBitmap* bitmap);
        void setShader(ImageShader* shader);

        AbstractBitmap* getOutputBitmap() const { return output; }
        ImageShader* getShader()          const { return shader; }
        const size_t getSamplersCount()   const { return samplers.size(); }
    };
}

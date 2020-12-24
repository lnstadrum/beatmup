/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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
#include "../parallelism.h"
#include "../bitmap/abstract_bitmap.h"
#include "../shading/image_shader.h"

namespace Beatmup {
    /**
        Image filtering operations
    */
    namespace Filters {

        /**
            Base class for image filters processing a given bitmap in a pixelwise fashion.
         */
        class PixelwiseFilter : public AbstractTask, private BitmapContentLock {
        protected:
            static const std::string GLSL_RGBA_INPUT;

            AbstractBitmap *inputBitmap, *outputBitmap;
            ImageShader *shader;

            /**
                Applies filtering to given pixel data.
                \param x            Horizontal position of the first pixel in image
                \param y            Vertical position of the first pixel in image
                \param nPix         Number of pixels to process
                \param thread       Calling thread descriptor
            */
            virtual void apply(int x, int y, msize nPix, TaskThread& thread) = 0;

            /**
                Provides GLSL declarations used in the GLSL code.
            */
            virtual std::string getGlslDeclarations() const;

            /**
                Provides GLSL source code of the filter.
            */
            virtual std::string getGlslSourceCode() const = 0;

            /**
                A callback run every time before the filter is applied to the image.
                \param useGpu       If `true`, the filter will be run on GPU.
            */
            virtual void setup(bool useGpu);

            virtual bool process(TaskThread& thread) final;
            virtual bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) final;
            virtual void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
            virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
            virtual TaskDeviceRequirement getUsedDevices() const final;

            PixelwiseFilter();
        public:
            virtual ~PixelwiseFilter();

            inline virtual void setInput(AbstractBitmap *input) { this->inputBitmap = input; }
            inline virtual void setOutput(AbstractBitmap *output) { this->outputBitmap = output; }

            inline AbstractBitmap *getInput() { return inputBitmap; }
            inline AbstractBitmap *getOutput() { return outputBitmap; }

            ThreadIndex getMaxThreads() const;
        };

    }

}

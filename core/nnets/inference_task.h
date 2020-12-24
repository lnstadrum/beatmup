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

#include "model.h"
#include "../gpu/gpu_task.h"
#include <map>

namespace Beatmup {
    namespace NNets {

        /**
            Task running inference of a Model.
            During the firs run of this task with a given model the shader programs are built and the memory is allocated.
            The subsequent runs are much faster.
        */
        class InferenceTask : public GpuTask, private BitmapContentLock {
        private:
            std::map<std::pair<AbstractOperation*, int>, AbstractBitmap*> inputImages;

            void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) override;
            void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) override;
            bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) override;
            bool process(TaskThread& thread) override;
            ThreadIndex getMaxThreads() const override { return MAX_THREAD_INDEX; }

        protected:
            ChunkCollection& data;
            Model& model;

        public:
            inline InferenceTask(Model& model, ChunkCollection& data): model(model), data(data) {}

            /**
                Connects an image to a specific operation input.
                Ensures the image content is up-to-date in GPU memory by the time the inference is run.
                \param[in] image            The image
                \param[in] operation        The operation
                \param[in] inputIndex       The input index of the operation
            */
            void connect(AbstractBitmap& image, AbstractOperation& operation, int inputIndex = 0);
            inline void connect(AbstractBitmap& image, const std::string& operation, int inputIndex = 0) {
                connect(image, model.getOperation(operation), inputIndex);
            }
        };

    }
}
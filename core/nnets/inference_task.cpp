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

#include "inference_task.h"

using namespace Beatmup;
using namespace NNets;


void InferenceTask::connect(AbstractBitmap& image, AbstractOperation& operation, int inputIndex) {
    inputImages[std::make_pair(&operation, inputIndex)] = &image;
    operation.setInput(image, inputIndex);
}


void InferenceTask::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    for (auto it : inputImages)
        readLock(gpu, it.second, ProcessingTarget::GPU);
    model.prepare(*gpu, data);
}


void InferenceTask::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    if (gpu)
        gpu->flush();
    unlockAll();
}


bool InferenceTask::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    model.execute(thread, &gpu);
    return true;
}


bool InferenceTask::process(TaskThread& thread) {
    model.execute(thread, nullptr);
    return true;
}

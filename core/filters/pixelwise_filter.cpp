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

#include "pixelwise_filter.h"
#include "../exception.h"

using namespace Beatmup;

const std::string Filters::PixelwiseFilter::GLSL_RGBA_INPUT("input");


Filters::PixelwiseFilter::PixelwiseFilter() :
    inputBitmap(nullptr), outputBitmap(nullptr), shader(nullptr)
{}


Filters::PixelwiseFilter::~PixelwiseFilter() {
    delete shader;
}


ThreadIndex Filters::PixelwiseFilter::getMaxThreads() const {
    NullTaskInput::check(inputBitmap, "input bitmap");
    // if there are few pixels, do not use many threads
    static const int MIN_PIXEL_COUNT_PER_THREAD = 64;
    return AbstractTask::validThreadCount(inputBitmap->getWidth() * inputBitmap->getHeight() / MIN_PIXEL_COUNT_PER_THREAD);
}


void Filters::PixelwiseFilter::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    NullTaskInput::check(inputBitmap, "input bitmap");
    NullTaskInput::check(outputBitmap, "output bitmap");
    RuntimeError::check(
        inputBitmap->getWidth() == outputBitmap->getWidth() &&
        inputBitmap->getHeight() <= outputBitmap->getHeight(),
        "Incompatible input and output bitmaps sizes");

    // use GPU is available and the input bitmap is up to date on GPU
    const bool useGpu = (target == ProcessingTarget::GPU && inputBitmap->isUpToDate(ProcessingTarget::GPU));

    // deferred shader preparation
    if (useGpu) {
        RuntimeError::check(inputBitmap->getContext() == outputBitmap->getContext(),
            "Input and output bitmaps are not attached to the same context");
        if (shader && !shader->usesContext(inputBitmap->getContext())) {
            delete shader;
            shader = nullptr;
        }
        if (!shader) {
            shader = new ImageShader(inputBitmap->getContext());
            shader->setSourceCode(ImageShader::CODE_HEADER + 
                getGlslDeclarations() + "\n" +
                "void main() {" +
                "lowp vec4 " +GLSL_RGBA_INPUT+ " = texture2D(" +
                        ImageShader::INPUT_IMAGE_ID + ", " +
                        Beatmup::GL::RenderingPrograms::TEXTURE_COORDINATES_ID +
                    ");\n" +
                    getGlslSourceCode() +
                "}"
            );
        }
    }

    // lock bitmaps content
    lock(gpu, useGpu ? ProcessingTarget::GPU : ProcessingTarget::CPU, inputBitmap, outputBitmap);

    // setup
    setup(useGpu);
}


void Filters::PixelwiseFilter::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    unlock(inputBitmap, outputBitmap);
}


bool Filters::PixelwiseFilter::process(TaskThread& thread) {
    // each thread receives a single part of bitmap to deal with
    const int w = inputBitmap->getWidth();
    const msize
        nPix = w * inputBitmap->getHeight(),
        start = nPix * thread.currentThread() / thread.numThreads(),
        stop  = nPix * (1 + thread.currentThread()) / thread.numThreads();

    apply((int)(start % w), (int)(start / w), stop - start, thread);
    return true;
}


bool Filters::PixelwiseFilter::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    shader->prepare(gpu, inputBitmap, outputBitmap);
    shader->process(gpu);
    return true;
}


AbstractTask::TaskDeviceRequirement Filters::PixelwiseFilter::getUsedDevices() const {
    NullTaskInput::check(inputBitmap, "input bitmap");
    return  inputBitmap->isUpToDate(ProcessingTarget::GPU) ?
        TaskDeviceRequirement::GPU_ONLY : TaskDeviceRequirement::CPU_ONLY;
}


std::string Filters::PixelwiseFilter::getGlslDeclarations() const {
    return "";
}


void Filters::PixelwiseFilter::setup(bool useGpu) {
    // nothing to do by default
}
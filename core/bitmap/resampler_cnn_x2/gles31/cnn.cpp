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

#include "cnn.h"
#include <algorithm>

#ifdef ENABLE_PROFILING
#include "../../../utils/profiler.h"
#include <iostream>
#endif

using namespace Beatmup;


#ifndef BEATMUP_OPENGLVERSION_GLES20

void Beatmup::GLES31X2UpsamplingNetwork::Layer::prepare(GraphicPipeline& gpu, GL::TextureHandler* input) {
    if (!prepared || (input && inputFormat != input->getTextureFormat())) {
        std::string code = "#version 310 es\n";

        if (input) {
            switch (inputFormat = input->getTextureFormat()) {
            case GL::TextureHandler::TextureFormat::Rx8:
            case GL::TextureHandler::TextureFormat::RGBx8:
            case GL::TextureHandler::TextureFormat::RGBAx8:
            case GL::TextureHandler::TextureFormat::Rx32f:
            case GL::TextureHandler::TextureFormat::RGBx32f:
            case GL::TextureHandler::TextureFormat::RGBAx32f:
                code += "#define beatmupSampler sampler2D\n";
                break;
            case GL::TextureHandler::TextureFormat::OES_Ext:
                code +=
                    "#extension GL_OES_EGL_image_external_essl3 : require\n"
                    "#define beatmupSampler samplerExternalOES\n"
                    "#define texelFetch texture\n";
                break;
            default:
                throw UnsupportedTextureFormat(inputFormat);
            }
        }

        code +=  "layout(local_size_x = " + std::to_string(wgSize[0]) + ", local_size_y = " + std::to_string(wgSize[1]) + ", local_size_z = " + std::to_string(wgSize[2]) + ") in;\n";

        program->make(gpu, code + sourceCodeTemplate);
        prepared = true;
    }

}

GLES31X2UpsamplingNetwork::Layer::Layer(
    GraphicPipeline& gpu, GL::RecycleBin& recycleBin, BitmapContentLock& lock, std::string sourceCodeTemplate,
    int inputZDim, int outputZDim, bool pointwise
):
    recycleBin(recycleBin), lock(lock), sourceCodeTemplate(sourceCodeTemplate), numInputs(inputZDim / 4), numOutputs(outputZDim / 4), prepared(false)
{
    program = new GL::ComputeProgram(gpu);

    wgSize[2] = 1;

    const unsigned int totalLimit = (unsigned int)gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_TOTAL);
    const unsigned int yLimit = std::min<unsigned int>(
        std::sqrt(totalLimit / wgSize[2]),
        gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_Y)
    );

    for (wgSize[1] = 1; wgSize[1] <= yLimit; wgSize[1] <<= 1);

    wgSize[0] = std::min<unsigned int>(
        totalLimit / (wgSize[1] * wgSize[2]),
        gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_X)
    );
}


GLES31X2UpsamplingNetwork::Layer::~Layer() {
    recycleBin.put(program);
}


void GLES31X2UpsamplingNetwork::Layer::process(GraphicPipeline& gpu, GL::TextureHandler& input, InternalBitmap** outputs) {
    prepare(gpu, &input);

    // enable
    program->enable(gpu);

    // bind outputs
    for (int i = 0; i < numOutputs; ++i) {
        if (outputs[i]->getWidth() != input.getWidth() || outputs[i]->getHeight() != input.getHeight())
            outputs[i]->reshape(input.getWidth(), input.getHeight());
        lock.writeLock(&gpu, outputs[i], ProcessingTarget::GPU);
        gpu.bind(*outputs[i], i, false, true);
    }

    // bind input
    program->setInteger("image", 0);
    gpu.bind(input, 0, TextureParam::INTERP_NEAREST);

    // g-g-go
    const int
        xWorkgroups = ceili(input.getWidth(), wgSize[0]),
        yWorkgroups = ceili(input.getHeight(), wgSize[1]);
    program->dispatch(gpu, xWorkgroups, yWorkgroups, 1);

    // unlock outputs
    for (int i = 0; i < numOutputs; ++i)
        lock.unlock(outputs[i]);
}


unsigned int GLES31X2UpsamplingNetwork::Layer::process(GraphicPipeline& gpu, InternalBitmap** inputs, GL::StorageBuffer& output, int numOutputParts) {
    prepare(gpu);

    // enable program
    program->enable(gpu);

    // bind inputs
    int bindingCtr = 0;
    for (int i = 0; i < numInputs; ++i) {
        InternalBitmap* input = inputs[i];
        lock.readLock(&gpu, input, ProcessingTarget::GPU);
        gpu.bind(*input, i, TextureParam::INTERP_NEAREST);
    }
    program->setIntegerArray("inFeatures", 0, numInputs);

    // bind output
    const unsigned int
        xWorkgroups = ceili(inputs[0]->getWidth(), wgSize[0]),
        yWorkgroups = ceili(inputs[0]->getHeight(), wgSize[1]);
    const unsigned int outputSize = xWorkgroups * wgSize[0] * yWorkgroups * wgSize[1] * (numOutputs * numOutputParts * 4);
    if (output.getCurrentCapacity() < outputSize)
        output.allocate(gpu, outputSize);
    output.bind(gpu, 0);

    // g-g-go
    program->dispatch(gpu, xWorkgroups, yWorkgroups, 1);

    // unlock
    for (int i = 0; i < numInputs; ++i)
        lock.unlock(inputs[i]);

    return xWorkgroups * wgSize[0];
}


void GLES31X2UpsamplingNetwork::Layer::processPointwise(GraphicPipeline& gpu, GL::StorageBuffer& input, unsigned int inputStridePix, InternalBitmap** outputs, int width, int height) {
    prepare(gpu);

    // enable program
    program->enable(gpu);

    // bind inputs
    input.bind(gpu, 0);
    program->setUnsignedInteger("inputStride", inputStridePix);

    // bind outputs
    for (int i = 0; i < numOutputs; ++i) {
        if (outputs[i]->getWidth() != width || outputs[i]->getHeight() != height)
            outputs[i]->reshape(width, height);
        lock.writeLock(&gpu, outputs[i], ProcessingTarget::GPU);
        gpu.bind(*outputs[i], i, false, true);
    }

    // g-g-go
    const unsigned int
        xWorkgroups = ceili(width, wgSize[0]),
        yWorkgroups = ceili(height, wgSize[1]);
    program->dispatch(gpu, xWorkgroups, yWorkgroups, 1);

    // unlock
    for (int i = 0; i < numOutputs; ++i)
        lock.unlock(outputs[i]);
}


void GLES31X2UpsamplingNetwork::Layer::processPointwise(GraphicPipeline& gpu, GL::StorageBuffer& inputFeatures, unsigned int inputStridePix, GL::TextureHandler& inputImage, AbstractBitmap& output) {
    prepare(gpu, &inputImage);

    // enable program
    program->enable(gpu);

    // bind outputs
    gpu.bind(output, 1, false, true);

    // bind inputs
    inputFeatures.bind(gpu, 0);
    gpu.bind(inputImage, 0, TextureParam::INTERP_LINEAR);
    program->setUnsignedInteger("inputStride", inputStridePix);
    program->setVector2("d1", 1.0f / inputImage.getWidth(), 1.0f / inputImage.getHeight());

    // g-g-go
    const unsigned int
        xWorkgroups = ceili(inputImage.getWidth(), wgSize[0]),
        yWorkgroups = ceili(inputImage.getHeight(), wgSize[1]);
    program->dispatch(gpu, xWorkgroups, yWorkgroups, 1);
}


void GLES31X2UpsamplingNetwork::process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output) {
    // disable alpha blend
    gpu.switchMode(GraphicPipeline::Mode::INFERENCE);

#ifdef ENABLE_PROFILING
    Profiler profiler;
    profiler("layer 1");
#endif

    layer1_0.process(gpu, input, storage + 0);
    layer1_1.process(gpu, input, storage + 6);

#ifdef ENABLE_PROFILING
    gpu.flush();
    profiler.lap();
    profiler("layer 2");
#endif

    layer2_0.process(gpu, storage,     buffer, 4);
    layer2_1.process(gpu, storage + 3, buffer, 4);
    layer2_2.process(gpu, storage + 6, buffer, 4);
    unsigned int stride = layer2_3.process(gpu, storage + 9, buffer, 4);

#ifdef ENABLE_PROFILING
    gpu.flush();
    profiler.lap();
    profiler("layer 3");
#endif

    layer3.processPointwise(gpu, buffer, stride, storage, input.getWidth(), input.getHeight());

#ifdef ENABLE_PROFILING
    gpu.flush();
    profiler.lap();
    profiler("layer 4");
#endif

    layer4_0.process(gpu, storage, buffer, 2);
    stride = layer4_1.process(gpu, storage + 3, buffer, 2);

#ifdef ENABLE_PROFILING
    gpu.flush();
    profiler.lap();
    profiler("layer 5");
#endif

    layer5.processPointwise(gpu, buffer, stride, input, output);

    gpu.flush();
#ifdef ENABLE_PROFILING
    profiler.lap();
    profiler.report(std::cout);
#endif
}


GLES31X2UpsamplingNetwork::GLES31X2UpsamplingNetwork(Context& ctx, GraphicPipeline& gpu) :
#define STRINGIFY(...) #__VA_ARGS__

    layer1_0(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l1-0.glsl"
        , 1, 24
    ),
    layer1_1(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l1-1.glsl"
        , 1, 24
    ),

    layer2_0(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l2-0.glsl"
        , 12, 8
    ),
    layer2_1(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l2-1.glsl"
        , 12, 8
    ),
    layer2_2(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l2-2.glsl"
        , 12, 8
    ),
    layer2_3(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l2-3.glsl"
        , 12, 8
    ),

    layer3(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l3.glsl"
        , 32, 24, true
    ),

    layer4_0(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l4-0.glsl"
        , 12, 8
    ),
    layer4_1(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l4-1.glsl"
        , 12, 8
    ),

    layer5(gpu, *ctx.getGpuRecycleBin(), *this,
#include "l5.glsl"
        , 16, 1, true
    ),

    buffer(*ctx.getGpuRecycleBin())
{
    for (int i = 0; i < STORAGE_SIZE; ++i)
        storage[i] = new InternalBitmap(ctx, PixelFormat::QuadByte, 64, 64, false);
}


GLES31X2UpsamplingNetwork::~GLES31X2UpsamplingNetwork() {
    for (int i = 0; i < STORAGE_SIZE; ++i)
        delete storage[i];
}

#endif

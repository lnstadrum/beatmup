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
        std::string code = BEATMUP_SHADER_HEADER_VERSION
            "layout(local_size_x = " + std::to_string(wgSize[0]) + ", local_size_y = " + std::to_string(wgSize[1]) + ", local_size_z = " + std::to_string(wgSize[2]) + ") in;\n";

        if (input) {
            switch (inputFormat = input->getTextureFormat()) {
            case GL::TextureHandler::TextureFormat::Rx8:
            case GL::TextureHandler::TextureFormat::RGBx8:
            case GL::TextureHandler::TextureFormat::RGBAx8:
            case GL::TextureHandler::TextureFormat::Rx32f:
            case GL::TextureHandler::TextureFormat::RGBx32f:
            case GL::TextureHandler::TextureFormat::RGBAx32f:
                code += "uniform sampler2D image;\n";
                break;
            case GL::TextureHandler::TextureFormat::OES_Ext:
                code +=
                    "#extension GL_OES_EGL_image_external : require\n"
                    "uniform samplerExternalOES image\n";
                break;
            default:
                throw UnsupportedTextureFormat(inputFormat);
            }
        }

        program->make(gpu, code + sourceCodeTemplate);
        prepared = true;
    }

}

GLES31X2UpsamplingNetwork::Layer::Layer(
    GraphicPipeline& gpu, GL::RecycleBin& recycleBin, std::string sourceCodeTemplate,
    int inputZDim, int outputZDim, bool pointwise
):
    recycleBin(recycleBin), sourceCodeTemplate(sourceCodeTemplate), numInputs(inputZDim / 4), numOutputs(outputZDim / 4), prepared(false)
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
    class Deleter : public GL::RecycleBin::Item {
        GL::ComputeProgram* program;
    public:
        Deleter(GL::ComputeProgram* program) : program(program) {}
        ~Deleter() {
            if (program)
                delete program;
        }
    };
    recycleBin.put(new Deleter(program));
}


void GLES31X2UpsamplingNetwork::Layer::process(GraphicPipeline& gpu, GL::TextureHandler& input, InternalBitmap** outputs) {
    prepare(gpu, &input);

    // enable
    program->enable(gpu);

    // bind outputs
    for (int i = 0; i < numOutputs; ++i) {
        if (outputs[i]->getWidth() != input.getWidth() || outputs[i]->getHeight() != input.getHeight())
            outputs[i]->reshape(input.getWidth(), input.getHeight());
        outputs[i]->lockContent(PixelFlow::GpuWrite);
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
        outputs[i]->unlockContent(PixelFlow::GpuWrite);
}


unsigned int GLES31X2UpsamplingNetwork::Layer::process(GraphicPipeline& gpu, InternalBitmap** inputs, GL::StorageBuffer& output, int numOutputParts) {
    prepare(gpu);

    // enable program
    program->enable(gpu);

    // bind inputs
    int bindingCtr = 0;
    for (int i = 0; i < numInputs; ++i) {
        InternalBitmap* input = inputs[i];
        input->lockContent(PixelFlow::GpuRead);
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
        inputs[i]->unlockContent(PixelFlow::GpuRead);

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
        outputs[i]->lockContent(PixelFlow::GpuWrite);
        gpu.bind(*outputs[i], i, false, true);
    }

    // g-g-go
    const unsigned int
        xWorkgroups = ceili(width, wgSize[0]),
        yWorkgroups = ceili(height, wgSize[1]);
    program->dispatch(gpu, xWorkgroups, yWorkgroups, 1);

    // unlock
    for (int i = 0; i < numOutputs; ++i)
        outputs[i]->unlockContent(PixelFlow::GpuWrite);
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
    gpu.switchAlphaBlending(false);

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
#undef clamp
    layer1_0(gpu, *ctx.getGpuRecycleBin(),
#include "l1-0.glsl"
        , 1, 24
    ),
    layer1_1(gpu, *ctx.getGpuRecycleBin(),
#include "l1-1.glsl"
        , 1, 24
    ),

    layer2_0(gpu, *ctx.getGpuRecycleBin(),
#include "l2-0.glsl"
        , 12, 8
    ),
    layer2_1(gpu, *ctx.getGpuRecycleBin(),
#include "l2-1.glsl"
        , 12, 8
    ),
    layer2_2(gpu, *ctx.getGpuRecycleBin(),
#include "l2-2.glsl"
        , 12, 8
    ),
    layer2_3(gpu, *ctx.getGpuRecycleBin(),
#include "l2-3.glsl"
        , 12, 8
    ),

    layer3(gpu, *ctx.getGpuRecycleBin(),
#include "l3.glsl"
        , 32, 24, true
    ),

    layer4_0(gpu, *ctx.getGpuRecycleBin(),
#include "l4-0.glsl"
        , 12, 8
    ),
    layer4_1(gpu, *ctx.getGpuRecycleBin(),
#include "l4-1.glsl"
        , 12, 8
    ),

    layer5(gpu, *ctx.getGpuRecycleBin(),
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

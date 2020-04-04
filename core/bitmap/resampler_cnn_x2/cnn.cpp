#include "cnn.h"

using namespace Beatmup;


X2UpsamplingNetwork::Layer::Layer(GL::RecycleBin& recycleBin, GraphicPipeline& gpu, Storage& outputStorage, const char* sourceCode):
  shader(recycleBin), output(outputStorage)
{
    shader.setSourceCode(sourceCode);
}


void X2UpsamplingNetwork::Layer::process(Context& ctx, GraphicPipeline& gpu, GL::TextureHandler& input) {
    if (output && (output->getWidth() != input.getWidth() || output->getHeight() != input.getHeight())) {
        delete output;
        output = nullptr;
    }
    if (!output) {
        output = new InternalBitmap(ctx, PixelFormat::QuadByte, input.getWidth(), input.getHeight(), false);
        shader.setFloat("d1", 1.0 / input.getWidth(), 1.0 / input.getHeight());
        shader.setFloat("d2", 2.0 / input.getWidth(), 2.0 / input.getHeight());
    }

    AbstractBitmap::ContentLock outputLock(*output, PixelFlow::GpuWrite);
    shader.prepare(gpu, &input, TextureParam::INTERP_NEAREST, output, AffineMapping::IDENTITY);
    shader.process(gpu);
}


void X2UpsamplingNetwork::Layer::process(Context& ctx, GraphicPipeline& gpu, Layer** inputs, int inputsCount) {
    const InternalBitmap& input = inputs[0]->getOutput();
    if (output && (output->getWidth() != input.getWidth() || output->getHeight() != input.getHeight())) {
        delete output;
        output = nullptr;
    }
    if (!output)
        output = new InternalBitmap(ctx, PixelFormat::QuadByte, input.getWidth(), input.getHeight(), false);

    AbstractBitmap::ContentLock outputLock(*output, PixelFlow::GpuWrite);
    shader.setFloat("d1", 1.0 / input.getWidth(), 1.0 / input.getHeight());
    shader.prepare(gpu, nullptr, output);

    // bind images
    for (int i = 0; i < inputsCount; ++i)
        gpu.bind(inputs[i]->getOutput(), i, TextureParam::INTERP_NEAREST);
    shader.bindSamplerArray("images", 0, inputsCount);

    // process
    shader.process(gpu);
}


void X2UpsamplingNetwork::process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output) {
    // disable alpha blend
    gpu.switchAlphaBlending(false);

    Context& ctx = output.getContext();
    for (int i = 0; i < L1_SIZE; ++i)
        layer1[i]->process(ctx, gpu, input);

    layer2[0]->process(ctx, gpu, layer1 + 0, 3);
    layer2[1]->process(ctx, gpu, layer1 + 0, 3);
    layer2[2]->process(ctx, gpu, layer1 + 3, 3);
    layer2[3]->process(ctx, gpu, layer1 + 3, 3);
    layer2[4]->process(ctx, gpu, layer1 + 6, 3);
    layer2[5]->process(ctx, gpu, layer1 + 6, 3);
    layer2[6]->process(ctx, gpu, layer1 + 9, 3);
    layer2[7]->process(ctx, gpu, layer1 + 9, 3);

    for (int i = 0; i < L3_SIZE; ++i)
        layer3[i]->process(ctx, gpu, layer2, L2_SIZE);

    layer4[0]->process(ctx, gpu, layer3 + 0, 3);
    layer4[1]->process(ctx, gpu, layer3 + 0, 3);
    layer4[2]->process(ctx, gpu, layer3 + 3, 3);
    layer4[3]->process(ctx, gpu, layer3 + 3, 3);

    layer5->process(ctx, gpu, layer4, L4_SIZE);

    demux.setInteger("convnetOutput", 1);
    demux.prepare(gpu, &input, TextureParam::INTERP_LINEAR, &output, AffineMapping::IDENTITY);
    gpu.bind(layer5->getOutput(), 1, TextureParam::INTERP_NEAREST);
    demux.process(gpu);

    gpu.flush();
}


X2UpsamplingNetwork::X2UpsamplingNetwork(GL::RecycleBin& recycleBin, GraphicPipeline& gpu):
    demux(recycleBin)
{
    for (int i = 0; i < STORAGE_SIZE; ++i)
        storage[i] = nullptr;

    int i = 0;
#define STRINGIFY(...) BEATMUP_SHADER_CODE(__VA_ARGS__)
#undef clamp

    layer1[0] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__0.glsl"
    );

    layer1[1] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__1.glsl"
    );

    layer1[2] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__2.glsl"
    );

    layer1[3] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__3.glsl"
    );

    layer1[4] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__4.glsl"
    );

    layer1[5] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__5.glsl"
    );

    layer1[6] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__6.glsl"
    );

    layer1[7] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__7.glsl"
    );

    layer1[8] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__8.glsl"
    );

    layer1[9] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__9.glsl"
    );

    layer1[10] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__10.glsl"
    );

    layer1[11] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l1__11.glsl"
    );


    layer2[0] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-0__0.glsl"
    );

    layer2[1] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-0__1.glsl"
    );

    layer2[2] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-1__0.glsl"
    );

    layer2[3] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-1__1.glsl"
    );

    layer2[4] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-2__0.glsl"
    );

    layer2[5] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-2__1.glsl"
    );

    layer2[6] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-3__0.glsl"
    );

    layer2[7] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l2-3__1.glsl"
    );


    layer3[0] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l3__0.glsl"
    );

    layer3[1] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l3__1.glsl"
    );

    layer3[2] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l3__2.glsl"
    );

    layer3[3] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l3__3.glsl"
    );

    layer3[4] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l3__4.glsl"
    );

    layer3[5] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l3__5.glsl"
    );


    layer4[0] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l4-0__0.glsl"
    );

    layer4[1] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l4-0__1.glsl"
    );

    layer4[2] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l4-1__0.glsl"
    );

    layer4[3] = new Layer(recycleBin, gpu, nextStorage(i),
#include "l4-1__1.glsl"
    );


    layer5 = new Layer(recycleBin, gpu, nextStorage(i),
#include "l5.glsl"
    );

    demux.setSourceCode(
#include "ycbcr_demuxer.glsl"
    );
}


X2UpsamplingNetwork::~X2UpsamplingNetwork() {
    for (int i = 0; i < L1_SIZE; ++i)
        delete layer1[i];
    for (int i = 0; i < L2_SIZE; ++i)
        delete layer2[i];
    for (int i = 0; i < L3_SIZE; ++i)
        delete layer3[i];
    for (int i = 0; i < L4_SIZE; ++i)
        delete layer4[i];
    delete layer5;

    for (int i = 0; i < STORAGE_SIZE; ++i)
        if (storage[i])
            delete storage[i];
}

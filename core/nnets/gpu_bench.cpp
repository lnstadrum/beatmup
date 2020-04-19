#include "gpu_bench.h"
#include "../gpu/recycle_bin.h"
#include "../gpu/pipeline.h"
#include "../bitmap/bitmap_access.h"
#include "../debug.h"
#include <random>

using namespace Beatmup;
using namespace NNets;


static const char* BENCHMARK_PROGRAM_SOURCE = BEATMUP_SHADER_CODE_V(
    layout(local_size_x = 1, local_size_y = 1) in;
    layout(binding = 0, rgba32f) uniform highp readonly image2D inputTensor;
    layout(binding = 1, rgba32f) uniform highp writeonly image2D outputTensor;
    void main() {
        ivec2 p = ivec2(gl_GlobalInvocationID.xy);
        vec4 v =
            imageLoad(inputTensor, p + ivec2(-1, -1)) * vec4(0.1, 0.1, 0.1, 0.1) +
            imageLoad(inputTensor, p + ivec2(-1, 0 )) * vec4(0.2, 0.2, 0.2, 0.2) +
            imageLoad(inputTensor, p + ivec2(-1, +1)) * vec4(0.3, 0.3, 0.3, 0.3) +
            imageLoad(inputTensor, p + ivec2(0, -1 )) * vec4(0.4, 0.4, 0.4, 0.4) +
            imageLoad(inputTensor, p                ) * vec4(0.5, 0.5, 0.5, 0.5) +
            imageLoad(inputTensor, p + ivec2(0, +1 )) * vec4(0.6, 0.6, 0.6, 0.6) +
            imageLoad(inputTensor, p + ivec2(+1, -1)) * vec4(0.7, 0.7, 0.7, 0.7) +
            imageLoad(inputTensor, p + ivec2(+1, 0 )) * vec4(0.8, 0.8, 0.8, 0.8) +
            imageLoad(inputTensor, p + ivec2(+1, +1)) * vec4(0.9, 0.9, 0.9, 0.9);
        v = max(v - vec4(2.0, 2.0, 2.0, 2.0), vec4(0.0, 0.0, 0.0, 0.0));
        imageStore(outputTensor, p, v);
    }
);

static const int TEST_BITMAP_SIZE = 1024;


GPUBenchmark::GPUBenchmark(Context& ctx):
    ctx(ctx),
    program(nullptr),
    input(ctx, PixelFormat::QuadFloat, TEST_BITMAP_SIZE, TEST_BITMAP_SIZE),
    output(ctx, PixelFormat::QuadFloat, TEST_BITMAP_SIZE, TEST_BITMAP_SIZE),
    error(0),
    score(0)
{}


GPUBenchmark::~GPUBenchmark() {
    if (program)
        program->destroy(*ctx.getGpuRecycleBin());
}


void GPUBenchmark::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    // preparing program if not yet
    if (!program) {
        program = new GL::Object<GL::ComputeProgram>(*gpu);
        program->make(*gpu, BENCHMARK_PROGRAM_SOURCE);
    }

    {
        AbstractBitmap::WriteLock lock(input);
        QuadFloatBitmapWriter in(input);
        const msize cpStep = (TEST_BITMAP_SIZE - 2) * (TEST_BITMAP_SIZE - 2) / CONTROL_POINTS_NUM;

        for (msize i = 0; i < TEST_BITMAP_SIZE * TEST_BITMAP_SIZE; ++i, in++) {
            in = pixfloat4{ (float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX };
        }

        in.goTo(0, 0);
        for (msize y = 0, cp = 0, cpi = 0; y < TEST_BITMAP_SIZE; ++y)
            for (msize x = 0; x < TEST_BITMAP_SIZE; ++x, ++cp, in++) {
                if (cp >= cpStep && cpi < CONTROL_POINTS_NUM &&
                    x > 0 && y > 0 &&
                    x < TEST_BITMAP_SIZE - 1 && y < TEST_BITMAP_SIZE - 1)
                {
                    pixfloat4 v = (
                        in.at(-1, -1) * 0.1f +
                        in.at(-1, 0 ) * 0.2f +
                        in.at(-1, +1) * 0.3f +
                        in.at(0, -1 ) * 0.4f +
                        in()          * 0.5f +
                        in.at(0, +1 ) * 0.6f +
                        in.at(+1, -1) * 0.7f +
                        in.at(+1, 0 ) * 0.8f +
                        in.at(+1, +1) * 0.9f
                    ) - 2.0f;
                    v[0] = std::max(0.0f, v[0]);
                    v[1] = std::max(0.0f, v[1]);
                    v[2] = std::max(0.0f, v[2]);
                    v[3] = std::max(0.0f, v[3]);
                    ctrlPointsVals[cpi] = v.mean();
                    ctrlPointsLocs[cpi] = y * TEST_BITMAP_SIZE + x;
                    cpi++;
                    cp = 0;
                }
            }
    }

    input.lockContent(PixelFlow::GpuRead);
    output.lockContent(PixelFlow::GpuWrite);
}


void GPUBenchmark::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    input.unlockContent(PixelFlow::GpuRead);
    output.unlockContent(PixelFlow::GpuWrite);
    gpu->fetchPixels(output);
    AbstractBitmap::ReadLock lock(output, ProcessingTarget::CPU);
    error = 0;
    for (msize i = 0; i < CONTROL_POINTS_NUM; ++i) {
        QuadFloatBitmapReader read(output);
        read += ctrlPointsLocs[i];
        float d = std::abs(read().mean() - ctrlPointsVals[i]);
        if (d > error) {
            error = d;
        }
    }
}


bool GPUBenchmark::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    program->enable(gpu);
    gpu.bind(input, 0, true, false);
    gpu.bind(output, 1, false, true);
    auto startTime = std::chrono::high_resolution_clock::now();
    program->dispatch(gpu, TEST_BITMAP_SIZE, TEST_BITMAP_SIZE, 1);
    auto endTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    score = TEST_BITMAP_SIZE * TEST_BITMAP_SIZE * 4 / (time * 1e-3);
    return true;
}


void GPUBenchmark::run() {
    ctx.performTask(*this);
}

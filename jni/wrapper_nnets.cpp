#include "wrapper.h"

#include "jniheaders/Beatmup_NNets_GPUBenchmark.h"
#include "android/context.h"

#include <core/nnets/gpu_bench.h>
#include <core/nnets/model.h>
#include <core/nnets/ops/convs_2d.h>
#include <core/nnets/ops/pooling.h>
#include <core/nnets/inference.h>

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       GPUBenchmark
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newGpuBenchmark, Java_Beatmup_NNets_GPUBenchmark, newGpuBenchmark)(JNIEnv * jenv, jclass, jobject jEnv) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, jEnv);
    return (jlong) (new Beatmup::NNets::GPUBenchmark(*ctx));
}


JNIMETHOD(jfloat, getScore, Java_Beatmup_NNets_GPUBenchmark, getScore)(JNIEnv * jenv, jclass, jlong hBench) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::GPUBenchmark, bench, hBench);
    return (jfloat)bench->getScore();
}


JNIMETHOD(jfloat, getError, Java_Beatmup_NNets_GPUBenchmark, getError)(JNIEnv * jenv, jclass, jlong hBench) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::GPUBenchmark, bench, hBench);
    return (jfloat)bench->getError();
}


JNIMETHOD(jfloat, test, Java_Beatmup_NNets_GPUBenchmark, test)(JNIEnv * jenv, jclass, jobject jEnv, jobject jTest) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, jEnv);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, test, jTest);

    std::fstream teststr("/storage/emulated/0/testmodel.chunks", std::ios::in | std::ios::binary);
    LOG_I("test stream is %s\n", teststr.good() ? "OK" : "bad...");
    teststr.close();

    if (true){
        Beatmup::NNets::Model model("/storage/emulated/0/testmodel.chunks");
        model.addOperation(new Beatmup::NNets::Ops::ImageInput(*ctx, "in", Beatmup::NNets::Size(224, 224, 3)));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_0/Conv2D",
                Beatmup::NNets::Size(224, 224, 3), 32,
                Beatmup::NNets::Size(3, 3, 3),
                false,
                Beatmup::NNets::Size(2, 2, 1)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_1_depthwise/depthwise",
                Beatmup::NNets::Size(112, 112, 32), 32,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_1_pointwise/Conv2D",
                Beatmup::NNets::Size(112, 112, 32), 64,
                Beatmup::NNets::Size(1, 1, 32)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_2_depthwise/depthwise",
                Beatmup::NNets::Size(112, 112, 64), 64,
                Beatmup::NNets::Size(3, 3, 1),
                true,
                Beatmup::NNets::Size(2, 2, 1),
                Beatmup::NNets::Size::Padding::SAME
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_2_pointwise/Conv2D",
                Beatmup::NNets::Size(56, 56, 64), 128,
                Beatmup::NNets::Size(1, 1, 64)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_3_depthwise/depthwise",
                Beatmup::NNets::Size(56, 56, 128), 128,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_3_pointwise/Conv2D",
                Beatmup::NNets::Size(56, 56, 128), 128,
                Beatmup::NNets::Size(1, 1, 128)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_4_depthwise/depthwise",
                Beatmup::NNets::Size(56, 56, 128), 128,
                Beatmup::NNets::Size(3, 3, 1),
                true,
                Beatmup::NNets::Size(2, 2, 1)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_4_pointwise/Conv2D",
                Beatmup::NNets::Size(28, 28, 128), 256,
                Beatmup::NNets::Size(1, 1, 128),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_5_depthwise/depthwise",
                Beatmup::NNets::Size(28, 28, 256), 256,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_5_pointwise/Conv2D",
                Beatmup::NNets::Size(28, 28, 256), 256,
                Beatmup::NNets::Size(1, 1, 256),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_6_depthwise/depthwise",
                Beatmup::NNets::Size(28, 28, 256), 256,
                Beatmup::NNets::Size(3, 3, 1),
                true,
                Beatmup::NNets::Size(2, 2, 1)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_6_pointwise/Conv2D",
                Beatmup::NNets::Size(14, 14, 256), 512,
                Beatmup::NNets::Size(1, 1, 256),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_7_depthwise/depthwise",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_7_pointwise/Conv2D",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(1, 1, 512),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_8_depthwise/depthwise",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_8_pointwise/Conv2D",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(1, 1, 512),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_9_depthwise/depthwise",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_9_pointwise/Conv2D",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(1, 1, 512),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_10_depthwise/depthwise",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_10_pointwise/Conv2D",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(1, 1, 512),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_11_depthwise/depthwise",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_11_pointwise/Conv2D",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(1, 1, 512),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_12_depthwise/depthwise",
                Beatmup::NNets::Size(14, 14, 512), 512,
                Beatmup::NNets::Size(3, 3, 1),
                true,
                Beatmup::NNets::Size(2, 2, 1)
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_12_pointwise/Conv2D",
                Beatmup::NNets::Size(7, 7, 512), 1024,
                Beatmup::NNets::Size(1, 1, 512),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_13_depthwise/depthwise",
                Beatmup::NNets::Size(7, 7, 1024), 1024,
                Beatmup::NNets::Size(3, 3, 1),
                true
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/MobilenetV1/Conv2d_13_pointwise/Conv2D",
                Beatmup::NNets::Size(7, 7, 1024), 1024,
                Beatmup::NNets::Size(1, 1, 1024),
                false
        ));

        model.addOperation(new Beatmup::NNets::Ops::Pooling(
                *ctx, "MobilenetV1/Logits/AvgPool_1a/AvgPool",
                Beatmup::NNets::Ops::Pooling::Mode::AVERAGE,
                Beatmup::NNets::Size(7, 7, 1024), 1024,
                Beatmup::NNets::Size(7, 7, 1024),
                Beatmup::NNets::Size::ONES,
                Beatmup::NNets::Size::Padding::VALID
        ));

        model.addOperation(new Beatmup::NNets::Ops::Convolution2D(
                *ctx, "MobilenetV1/Logits/Conv2d_1c_1x1/Conv2D",
                Beatmup::NNets::Size(1, 1, 1024), 1001,
                Beatmup::NNets::Size(1, 1, 1024),
                false,
                Beatmup::NNets::Size::ONES,
                Beatmup::NNets::Size::Padding::SAME,
                Beatmup::NNets::Ops::Convolution2D::ActivationFunc::IDENTITY
        ));

        /*Beatmup::NNets::Probe* probe = new Beatmup::NNets::Probe(
            *ctx, "probe",
            Beatmup::NNets::Size(112, 112, 32), 7
        );
        model.addOperation(probe);*/

        Beatmup::NNets::Inference inference;
        //inference.enableCache("inference_cache.chunks");
        inference.setModel(&model);
        inference.setPrepareOnly(true);
        ctx->performTask(inference);

        LOG_I("INFERRING...");
        const Beatmup::NNets::Size outputSize = model.getLastAddedOp().getOutputSize();
        Beatmup::GL::StorageBuffer output(*ctx->getGpuRecycleBin());
        inference.supplyOutput(output, outputSize.volume() * sizeof(float), model.getLastAddedOp().getName());
        inference.supplyInput(*test, "in");
        test->lockContent(Beatmup::PixelFlow::GpuRead);
        float totalTime = 0;
        for (int it = 0; it < 10; ++it)
            totalTime += ctx->performTask(inference);
        test->unlockContent(Beatmup::PixelFlow::GpuRead);

        std::vector<float> data(outputSize[2]);
        Beatmup::GL::StorageBufferFetcher fetch(output, data.data(), data.size() * sizeof(float));
        ctx->performTask(fetch);
        for (int i = 0; i < 10; i++)
            LOG_I("%0.5f ", data[i]);

        LOG_I(" == %0.2f ms\n", totalTime / 10);
#ifdef BEATMUP_DEBUG
        std::ostringstream os;
        inference.getProfiler().report(os, Beatmup::Profiler::ReportType::BRIEF);
        LOG_I("%s\n\n", os.str().c_str());
        std::fstream report("/storage/emulated/0/report.txt", std::ios::out);
        report << os.str();
        report.close();
#endif
    }

    return 0;
}

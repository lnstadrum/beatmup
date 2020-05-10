#include "wrapper.h"

#include "jniheaders/Beatmup_NNets_GPUBenchmark.h"
#include "android/context.h"

#include <core/nnets/gpu_bench.h>

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


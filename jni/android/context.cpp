#include "context.h"

using namespace Beatmup;


Android::Context::Context(JNIEnv *jenv, const PoolIndex numThreadPools, const char *swapPrefix, const char *swapSuffix):
    Beatmup::Context(numThreadPools, swapPrefix, swapSuffix)
{}


Android::Context::~Context() {
}
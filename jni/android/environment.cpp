#include "environment.h"

using namespace Beatmup;


Android::Environment::Environment(JNIEnv *jenv, const PoolIndex numThreadPools, const char *swapPrefix, const char *swapSuffix):
    Beatmup::Environment(numThreadPools, swapPrefix, swapSuffix)
{}


Android::Environment::~Environment() {
}
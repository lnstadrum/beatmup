#pragma once

#include <core/environment.h>
#include <jni.h>

namespace Beatmup {
    namespace Android {

        class Environment : public Beatmup::Environment {
        public:
            Environment(JNIEnv *jenv, const Beatmup::PoolIndex numThreadPools, const char *swapPrefix, const char *swapSuffix);
            ~Environment();
        };

    }
}

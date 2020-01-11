#pragma once

#include <core/context.h>
#include <jni.h>

namespace Beatmup {
    namespace Android {

        class Context : public Beatmup::Context {
        public:
            Context(JNIEnv *jenv, const Beatmup::PoolIndex numThreadPools, const char *swapPrefix, const char *swapSuffix);
            ~Context();
        };

    }
}

#pragma once
#include <jni.h>
#include <core/environment.h>

namespace Beatmup {
    namespace Android {

        /**
         * Binding a SurfaceView to a Beatmup context
         */
        bool switchDisplay(Beatmup::Environment& env, JNIEnv* jenv, jobject surface);

    }
}
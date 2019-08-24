#pragma once

#include <core/environment.h>
#include <jni.h>
#include "camera_texture.h"
#include <mutex>

namespace Beatmup {
    namespace Android {

        class Environment : public Beatmup::Environment {
        public:
            bool cameraTextureUpdated;  //!< signals that an update has to be done for the camera texture
            CameraTexture *cameraTexture;

            Environment(JNIEnv *jenv, const Beatmup::PoolIndex numThreadPools, const char *swapPrefix, const char *swapSuffix);
            ~Environment();

            void initCameraTexture(JNIEnv *jenv, jobject context);
        };

    }
}
#include "environment.h"

using namespace Beatmup;


Android::Environment::Environment(JNIEnv *jenv, const PoolIndex numThreadPools, const char *swapPrefix, const char *swapSuffix):
    Beatmup::Environment(numThreadPools, swapPrefix, swapSuffix),
    cameraTextureUpdated(false),
    cameraTexture(NULL)
{}


Android::Environment::~Environment() {
    if (cameraTexture)
        delete cameraTexture;
}


void Android::Environment::initCameraTexture(JNIEnv *jenv, jobject context) {
    cameraTexture = CameraTexture::initCameraTexture(*this, jenv, context);
}
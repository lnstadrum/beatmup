//
// Created by HomePlaneR on 14/04/2016.
//

#include "camera_texture.h"

#include <core/environment.h>
#include <core/gpu/bgl.h>
using namespace Beatmup;
using namespace Android;

class CameraTextureInitializer : public AbstractTask {
private:
    Environment& env;
    JavaVM* jvm;
    jobject context;
    jmethodID callback;
    CameraTexture* cameraTexture;

    CameraTextureInitializer(Environment& env, JNIEnv* jenv, jobject context, jmethodID callback):
        context(context), callback(callback), env(env)
    {
        jenv->GetJavaVM(&jvm);
    }

    bool processOnGPU(GraphicPipeline& gpu, TaskThread&) {
        JNIEnv *jenv;
        jvm->AttachCurrentThread(&jenv, NULL);
        cameraTexture = new CameraTexture(gpu, env, jenv, context, callback);
        return true;
    }

    bool process(TaskThread& thread) {
        return false;
    }

    ExecutionTarget getExecutionTarget() const {
        return useGPU;
    }

public:
    inline static CameraTexture* run(Environment& env, JNIEnv* jenv, jobject object) {
        jclass cls = jenv->FindClass("Beatmup/Android/Context");
        jmethodID callback = jenv->GetMethodID(cls, "initCamTextureCallback", "(I)V");
        jenv->DeleteLocalRef(cls);
        jobject ref = jenv->NewGlobalRef(object);
        CameraTextureInitializer me(env, jenv, ref, callback);
        env.performTask(me);
        jenv->DeleteGlobalRef(ref);
        return me.cameraTexture;
    }
};


CameraTexture::CameraTexture(
        GraphicPipeline& gpu,
        Environment& env,
        JNIEnv* jenv,
        jobject context,
        jmethodID callback
) : env(env) {
    width = height = 0;
    glGenTextures(1, &textureHandle);
    jenv->CallVoidMethod(context, callback, textureHandle);
}


CameraTexture::~CameraTexture() {
    invalidate(*env.getGpuRecycleBin());
}


CameraTexture* CameraTexture::initCameraTexture(Environment& env, JNIEnv* jenv, jobject context) {
    return CameraTextureInitializer::run(env, jenv, context);
}
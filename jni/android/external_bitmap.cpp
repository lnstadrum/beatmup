#include "external_bitmap.h"
#include <core/environment.h>
#include <core/gpu/bgl.h>
#include <core/debug.h>


using namespace Beatmup;
using namespace Android;

class TextureHandleFactory : public AbstractTask {
private:
    Environment& env;
    Beatmup::GL::glhandle textureHandle;

    TextureHandleFactory(Environment& env):
        env(env)
    {}

    bool processOnGPU(GraphicPipeline& gpu, TaskThread&) {
        glGenTextures(1, &textureHandle);
        return true;
    }

    bool process(TaskThread& thread) {
        return false;
    }

    ExecutionTarget getExecutionTarget() const {
        return useGPU;
    }

public:
    inline static Beatmup::GL::glhandle makeHandle(Environment& env) {
        TextureHandleFactory me(env);
        env.performTask(me);
        return me.textureHandle;
    }
};


ExternalBitmap::ExternalBitmap(Environment& env) : AbstractBitmap(env) {
    width = height = 0;
    this->textureHandle = TextureHandleFactory::makeHandle(env);
    textureUpdated = false;
    persistentJEnv = nullptr;
    jvm = nullptr;
}


void ExternalBitmap::bind(JNIEnv* jenv, jobject frontend) {
    jenv->GetJavaVM(&jvm);

    // grab classes
    jclass externalBitmapClass = jenv->FindClass("Beatmup/Android/ExternalBitmap");
    jclass surfaceTextureClass = jenv->FindClass("android/graphics/SurfaceTexture");

#ifdef BEATMUP_DEBUG
    if (!externalBitmapClass) {
        BEATMUP_DEBUG_E("Cannot find class Beatmup.Android.ExternalImage");
        return;
    }

    if (!surfaceTextureClass) {
        BEATMUP_DEBUG_E("Cannot find class android.graphics.SurfaceTexture");
        return;
    }
#endif

    // grab field id
    jfieldID surfaceTextureFieldId = jenv->GetFieldID(externalBitmapClass, "surfaceTexture", "Landroid/graphics/SurfaceTexture;");
#ifdef BEATMUP_DEBUG
    if (!surfaceTextureFieldId) {
        BEATMUP_DEBUG_E("Cannot find field surfaceTexture in Beatmup.Android.ExternalImage");
        return;
    }
#endif

    // grab method ids
    jmethodID surfaceTextureConstructor = jenv->GetMethodID(surfaceTextureClass, "<init>", "(I)V");
    updateTexImageMethodId = jenv->GetMethodID(surfaceTextureClass, "updateTexImage", "()V");
#ifdef BEATMUP_DEBUG
    if (!surfaceTextureConstructor) {
        BEATMUP_DEBUG_E("Cannot find SurfaceTexture constructor");
        return;
    }

    if (!updateTexImageMethodId) {
        BEATMUP_DEBUG_E("Cannot find SurfaceTexture.updateTexImage method");
        return;
    }
#endif

    // create an instance of SurfaceTexture
    jobject newSurfaceTextureObject = jenv->NewObject(surfaceTextureClass, surfaceTextureConstructor, this->textureHandle);
#ifdef BEATMUP_DEBUG
    if (!newSurfaceTextureObject) {
        BEATMUP_DEBUG_E("Cannot create SurfaceTexture instance");
        return;
    }
#endif

    // keep a global reference
    surfaceTexture = jenv->NewGlobalRef(newSurfaceTextureObject);

    // set field value
    jenv->SetObjectField(frontend, surfaceTextureFieldId, newSurfaceTextureObject);

    // clean up
    jenv->DeleteLocalRef(externalBitmapClass);
    jenv->DeleteLocalRef(surfaceTextureClass);
}


ExternalBitmap::~ExternalBitmap() {
    if (jvm) {
        JNIEnv *jenv;
        jvm->AttachCurrentThread(&jenv, nullptr);
        jenv->DeleteGlobalRef(surfaceTexture);
    }
}


void ExternalBitmap::notifyUpdate(const int width, const int height) {
    this->width = width;
    this->height = height;
    this->textureUpdated = true;
}


void ExternalBitmap::prepare(GraphicPipeline& gpu) {
    // call updateTexImage()
    if (textureUpdated && jvm) {
        if (!persistentJEnv)
            jvm->AttachCurrentThread(&persistentJEnv, nullptr);
        persistentJEnv->CallVoidMethod(surfaceTexture, updateTexImageMethodId);
        textureUpdated = false;
    }
}

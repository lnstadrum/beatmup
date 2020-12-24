/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "external_bitmap.h"
#include <core/context.h>
#include <core/gpu/bgl.h>
#include <core/debug.h>


using namespace Beatmup;
using namespace Android;

class TextureHandleFactory : public AbstractTask {
private:
    Context& ctx;
    Beatmup::GL::handle_t textureHandle;

    TextureHandleFactory(Context& ctx):
        ctx(ctx)
    {}

    bool processOnGPU(GraphicPipeline& gpu, TaskThread&) {
        glGenTextures(1, &textureHandle);
        return true;
    }

    bool process(TaskThread& thread) {
        return false;
    }

    TaskDeviceRequirement getUsedDevices() const {
        return TaskDeviceRequirement::GPU_ONLY;
    }

public:
    inline static Beatmup::GL::handle_t makeHandle(Context& ctx) {
        TextureHandleFactory me(ctx);
        ctx.performTask(me);
        return me.textureHandle;
    }
};


ExternalBitmap::ExternalBitmap(Context& ctx) : AbstractBitmap(ctx) {
    width = height = 0;
    this->textureHandle = TextureHandleFactory::makeHandle(ctx);
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
    TextureHandler::prepare(gpu);

    // call updateTexImage()
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, this->textureHandle);
    if (textureUpdated && jvm) {
        if (!persistentJEnv)
            jvm->AttachCurrentThread(&persistentJEnv, nullptr);
        persistentJEnv->CallVoidMethod(surfaceTexture, updateTexImageMethodId);
        textureUpdated = false;
    }

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

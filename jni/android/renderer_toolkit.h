#pragma once
#include <core/scene/renderer.h>
#include <core/scene/rendering_context.h>
#include "environment.h"
#include <jni.h>

namespace Beatmup {
    namespace Android {

        /**
         * Wraps rendering event listener in order to provide camera callback
         */
        template<class Renderer>
        class RendererToolkit : public Renderer {
        private:
            class RenderingEventListener : public Beatmup::RenderingContext::EventListener {
            private:
                Environment &env;
                JavaVM *jvm;
                JNIEnv *persistentJEnv;
                jmethodID updateCallbackMethodId;
                jobject context;
            public:

                RenderingEventListener(Environment &env, JNIEnv *jenv, jobject context) :
                        env(env),
                        context(jenv->NewGlobalRef(context)),
                        persistentJEnv(NULL) {
                    jenv->GetJavaVM(&jvm);
                    jclass cls = jenv->FindClass("Beatmup/Android/Context");
                    updateCallbackMethodId = jenv->GetMethodID(cls, "updateCamTextureCallback", "()V");
                    jenv->DeleteLocalRef(cls);
                }


                ~RenderingEventListener() {
                    // deleting global reference to the context
                    JNIEnv *jenv;
                    jvm->AttachCurrentThread(&jenv, NULL);
                    jenv->DeleteGlobalRef(context);
                }

                inline void onRenderingStart() { }

                inline void onCameraFrameRendering(Beatmup::GL::TextureHandler *&cameraFrame) {
                    cameraFrame = env.cameraTexture;
                    if (env.cameraTextureUpdated) {
                        env.cameraTextureUpdated = false;
                        if (!persistentJEnv)
                            jvm->AttachCurrentThread(&persistentJEnv, NULL);
                        persistentJEnv->CallVoidMethod(context, updateCallbackMethodId);
                    }
                }

            };

            RenderingEventListener renderingEventListener;

        public:
            RendererToolkit(Environment &env, JNIEnv *jenv, jobject context) :
                    renderingEventListener(env, jenv, context) {
                Renderer::setRenderingEventListener(&renderingEventListener);
            }

            virtual ~RendererToolkit() {
                Renderer::setRenderingEventListener(NULL);
            }
        };

    }
}
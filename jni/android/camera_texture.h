/**
 * Represents camera texture
 */

#pragma once
#include <core/gpu/pipeline.h>
#include <jni.h>


namespace Beatmup {
    namespace Android {

        class CameraTexture : public Beatmup::GL::TextureHandler {
        private:
            int width, height;
            Beatmup::Environment& env;
        public:
            /**
             * Instantiates camera texture; this may be done only in the environment managing thread
             */
            CameraTexture(Beatmup::GraphicPipeline& gpu, Beatmup::Environment& env, JNIEnv* jenv, jobject context, jmethodID callback);
            ~CameraTexture();

            const Beatmup::GL::TextureHandler::TextureFormat getTextureFormat() const {
                return Beatmup::GL::TextureHandler::TextureFormat::OES_Ext;
            }

            const int getWidth() const {
                return width;
            }

            const int getHeight() const {
                return height;
            }

            const int getDepth() const {
                return 1;
            }

            void setFrameSize(int width, int height) {
                this->width = width;
                this->height = height;
            }

            static CameraTexture* initCameraTexture(Beatmup::Environment& env, JNIEnv* jenv, jobject context);
        };

    }
}

/**
 * Image coming from a SurfaceTexture (Camera or video decoder)
 */

#pragma once
#include <core/bitmap/abstract_bitmap.h>
#include <jni.h>


namespace Beatmup {
    namespace Android {

        class ExternalBitmap : public Beatmup::AbstractBitmap {
        private:
            int width, height;
            JavaVM *jvm;
            JNIEnv* persistentJEnv;
            jmethodID updateTexImageMethodId;
            jobject surfaceTexture;
            bool textureUpdated;

        protected:
            virtual void prepare(GraphicPipeline& gpu, bool queryData);
            void lockPixelData() {}
            void unlockPixelData() {}

        public:
            /**
             * Instantiates external image.
             * This must not be called from Beatmup internal threads.
             */
            ExternalBitmap(Beatmup::Context& ctx);

            ~ExternalBitmap();

            /**
             * Attaches the bitmap object to its Java frontend and initializes
             * the surface texture in it.
             */
            void bind(JNIEnv* jenv, jobject frontend);

            const Beatmup::GL::TextureHandler::TextureFormat getTextureFormat() const {
                return Beatmup::GL::TextureHandler::TextureFormat::OES_Ext;
            }

            const PixelFormat getPixelFormat() const {
                return PixelFormat::TripleByte;
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

            const msize getMemorySize() const {
                return 0;
            }

            pixbyte* getData(int x, int y) const {
                return nullptr;
            }

            void notifyUpdate(const int width, const int height);
        };

    }
}

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

#pragma once
#include <core/bitmap/abstract_bitmap.h>
#include <jni.h>


namespace Beatmup {
    namespace Android {

        /**
            Image coming from a SurfaceTexture (Camera or video decoder)
        */
        class ExternalBitmap : public Beatmup::AbstractBitmap {
        private:
            int width, height;
            JavaVM *jvm;
            JNIEnv* persistentJEnv;
            jmethodID updateTexImageMethodId;
            jobject surfaceTexture;
            bool textureUpdated;

        protected:
            virtual void prepare(GraphicPipeline& gpu);
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

            const pixbyte* getData(int x, int y) const {
                return nullptr;
            }

            pixbyte* getData(int x, int y) {
                return nullptr;
            }

            void notifyUpdate(const int width, const int height);
        };

    }
}

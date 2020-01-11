/*
    3D tensor
*/

#pragma once

#include "../basic_types.h"
#include "pipeline.h"

namespace Beatmup {
    namespace GL {
        class Tensor : public TextureHandler {
            friend class GraphicPipeline;
        private:
            const TextureFormat format;
            const bool arrayTexture;
            const int width, height, depth;
            Context& ctx;
            bool allocated;

            void load(GraphicPipeline& gpu, int channel, const AbstractBitmap& bitmap);

            static TextureFormat getFormat(int scalarDepth);
        protected:
            void prepare(GraphicPipeline& gpu);

        public:
            Tensor(Context& ctx, const int width, const int height, const int scalarDepth);
            Tensor(Context& ctx, GraphicPipeline& gpu, const int unpackedWidth, const int height, const float* data);
            ~Tensor();

            const int getWidth()  const { return width; }
            const int getHeight() const { return height; }
            const int getDepth()  const { return depth; }
            const TextureFormat getTextureFormat() const { return format; }
        };
    }
}
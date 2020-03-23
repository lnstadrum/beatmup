/*
    x2 image upsampling using convolutional neural network inference on GPU
*/

#pragma once
#include "../internal_bitmap.h"
#include "../../shading/image_shader.h"
#include "../../gpu/pipeline.h"

namespace Beatmup {

    class X2UpsamplingNetwork {
    private:
        class Layer {
        private:
            ImageShader shader;
            InternalBitmap* output;
        public:
            Layer(GL::RecycleBin& recycleBin, GraphicPipeline& gpu, const char* sourceCode);
            ~Layer();

            void process(Context& ctx, GraphicPipeline& gpu, GL::TextureHandler& input);
            void process(Context& ctx, GraphicPipeline& gpu, Layer** inputs, int inputsCount);

            InternalBitmap& getOutput() {
                return *output;
            }
        };

        static const int
            L1_SIZE = 12,
            L2_SIZE = 8,
            L3_SIZE = 6,
            L4_SIZE = 4;

        Layer
            *layer1[L1_SIZE],
            *layer2[L2_SIZE],
            *layer3[L3_SIZE],
            *layer4[L4_SIZE],
            *layer5;

        ImageShader demux;

    public:
        X2UpsamplingNetwork(GL::RecycleBin& recycleBin, GraphicPipeline& gpu);
        ~X2UpsamplingNetwork();

        void process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output);
    };

}

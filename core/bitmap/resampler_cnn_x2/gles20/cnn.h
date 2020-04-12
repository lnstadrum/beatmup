/*
    x2 image upsampling using convolutional neural network inference on GPU
*/

#pragma once
#include "../cnn_interface.h"

namespace Beatmup {

    class GLES20X2UpsamplingNetwork : public X2UpsamplingNetwork {
    private:
        class Layer {
        public:
            typedef InternalBitmap* Storage;
        private:
            ImageShader shader;
            Storage& output;
        public:
            Layer(GL::RecycleBin& recycleBin, GraphicPipeline& gpu, Storage& outputStorage, const char* sourceCode);

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
            L4_SIZE = 4,
            STORAGE_SIZE = 14;    // min number of recyclable textures to cover the entire dataflow during the inference

        InternalBitmap* storage[STORAGE_SIZE];

        Layer
            *layer1[L1_SIZE],
            *layer2[L2_SIZE],
            *layer3[L3_SIZE],
            *layer4[L4_SIZE],
            *layer5;

        ImageShader demux;

        Layer::Storage& nextStorage(int& i) { return storage[i++ % STORAGE_SIZE]; }

    public:
        GLES20X2UpsamplingNetwork(GL::RecycleBin& recycleBin, GraphicPipeline& gpu);
        ~GLES20X2UpsamplingNetwork();

        void process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output);
    };

}

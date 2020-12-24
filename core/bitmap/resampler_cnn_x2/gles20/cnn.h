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
#include "../cnn_interface.h"

namespace Beatmup {

    /**
        x2 image upsampler using a convolutional neural network.
        Implements a neural net inference using OpenGL ES 2.0-conformant shaders.
        Used by Bitmap::Resampler. Only usable inside an AbstractTask, not intended to be directly used by the application.
    */
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
        bool usesEs31Backend() const { return false; }
    };

}

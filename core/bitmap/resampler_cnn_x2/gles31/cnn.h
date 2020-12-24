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
#include "../../../gpu/compute_program.h"
#include <string>

namespace Beatmup {

    /**
        x2 image upsampler using a convolutional neural network.
        Implements a neural net inference using OpenGL ES 3.1-conformant shaders.
        Used by Bitmap::Resampler. Only usable inside an AbstractTask, not intended to be directly used by the application.
    */
    class GLES31X2UpsamplingNetwork : public X2UpsamplingNetwork, private BitmapContentLock {
    private:
        static const int STORAGE_SIZE = 14;

        class Layer {
        private:
            GL::RecycleBin& recycleBin;
            GL::ComputeProgram* program;
            GL::TextureHandler::TextureFormat inputFormat;
            std::string sourceCodeTemplate;
            BitmapContentLock& lock;

            unsigned int wgSize[3];            // workgroup size along one axis
            const int numInputs, numOutputs;
            bool prepared;

            void prepare(GraphicPipeline& gpu, GL::TextureHandler* inputImage = nullptr);

        public:

            Layer(
                GraphicPipeline& gpu, GL::RecycleBin& recycleBin, BitmapContentLock& lock,
                std::string sourceCodeTemplate,
                int inputZDim, int outputZDim,
                bool pointwise = false
            );
            ~Layer();

            void process(GraphicPipeline& gpu, GL::TextureHandler& input, InternalBitmap** outputs);
            unsigned int process(GraphicPipeline& gpu, InternalBitmap** inputs, GL::StorageBuffer& output, int numOutputParts);
            void processPointwise(GraphicPipeline& gpu, GL::StorageBuffer& input, unsigned int inputStridePix, InternalBitmap** outputs, int width, int height);
            void processPointwise(GraphicPipeline& gpu, GL::StorageBuffer& inputFeatures, unsigned int inputStridePix, GL::TextureHandler& inputImage, AbstractBitmap& outputImage);
        };

        Layer
            layer1_0, layer1_1,
            layer2_0, layer2_1, layer2_2, layer2_3,
            layer3,
            layer4_0, layer4_1,
            layer5;

        InternalBitmap* storage[STORAGE_SIZE];
        GL::StorageBuffer buffer;

    public:
        GLES31X2UpsamplingNetwork(Context& ctx, GraphicPipeline& gpu);
        ~GLES31X2UpsamplingNetwork();

        void process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output);
        bool usesEs31Backend() const { return true; }
    };

}

/*
    x2 image upsampling using convolutional neural network inference on GPU
*/

#pragma once
#include "../cnn_interface.h"
#include "../../../gpu/compute_program.h"
#include <string>

namespace Beatmup {

    class GLES31X2UpsamplingNetwork : public X2UpsamplingNetwork {
    private:
        static const int STORAGE_SIZE = 14;

        class Layer {
        private:
            GL::RecycleBin& recycleBin;
            GL::ComputeProgram* program;
            GL::TextureHandler::TextureFormat inputFormat;
            std::string sourceCodeTemplate;

            unsigned int wgSize[3];            // workgroup size along one axis
            const int numInputs, numOutputs;
            bool prepared;

            void prepare(GraphicPipeline& gpu, GL::TextureHandler* inputImage = nullptr);

        public:

            Layer(
                GraphicPipeline& gpu, GL::RecycleBin& recycleBin,
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

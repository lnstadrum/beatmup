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

#include "operation.h"
#include "../gpu/texture_handler.h"
#include <vector>
#include <array>
#include <map>


namespace Beatmup {
    namespace NNets {

        /**
            2D convolution operation computed on GPU.
            Has 2 inputs: main and residual (detailed below), and a single output.
            Constraints:
                - Input and output contain values in [0, 1] range sampled over 8 bits.
                - Number of input channels is 3 (i.e., the input is an RGB image) or a multiple of 4.
                - Number of output feature maps is a multiple of 4.
                - For group convolutions, each group contains a multiple of 4 input channels and a multiple of 4 output
                  channels, or exactly 1 input and 1 output channel (i.e., depthwise).
                - Kernels are of square shape.
                - Strides are equal along X and Y.
                - Dilations are equal to 1.
                - If an image is given on input (3 input feature maps), only valid padding is supported.
                - An activation function is always applied on output.

            Raspberry Pi-related constraints:
                - Pi cannot sample more than 256 channels to compute a single output value. Actual practical limit is
                  yet lower: something about 128 channels for pointwise convolutions and less than 100 channels for
                  bigger kernels. When the limit is reached, Pi OpenGL driver reports an out of memory error (0x505).

            Features:
                - Bias addition integrated.
                - An optional residual input is available: a tensor of output shape added to the convolution result
                  before applying the activation function.

            Convolution filters and bias are searched in chunks. The chunk names consist of the operation name followed
            by Conv2D::FILTERS_CHUNK_SUFFIX and Conv2D::BIAS_CHUNK_SUFFIX respectively.
            The chunk contents is a single precision floating point arrays.
            The filter coefficients are taken in "OIHW" layout, i.e., there are 'O*I' contiguous packets of 'H*W'
            values each. "O" and "I" are output and input channel numbers, "H" and "W" are filter height and width.
        */
        class Conv2D :
            public AbstractOperation, protected SpatialFilteringMixin, protected ActivationFunctionMixin
        {
        private:
            const Size kernelSize;
            const int numOutputChannels;                    //!< number of output feature maps
            const int numGroups;                            //!< number of convolution groups
            const int stride;
            const Size::Padding padding;
            const bool useInputImage;                       //!< if `true`, input is the texture handler, not the view
            const bool isDepthwise;                         //!< if `true`, the convolution is depthwise, otherwise regular
            const bool useBias;                             //!< if `true`, the bias addition is enabled
            bool ready;

            Storage::View input, output;
            Storage::View residualInput;                    //!< optional tensor to be added to the output before activation
            GL::TextureHandler *inputImage;                 //!< input texture handler to be used instead input view
            std::vector<GL::RenderingProgram*> programs;    //!< pointers to GLSL program, one per quad of output channels
            std::vector<std::array<float, 4>> coeffs;       //!< model data to pass to uniform variables, if used
            std::vector<int> execOrder;                     //!< execution order of GLSL programs
            std::vector<Storage::View> groupViews;          //!< views per convolution group

            /**
                Maps an (inputChannel, outputChannel, x, y) position to a linear coefficient index in the chunkfile.
            */
            inline int getIdx(int output, int input, int x, int y) const {
                return output + numOutputChannels * (input + kernelSize[2] * (x + kernelSize[0] * y));
            }

            void prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank);
            void execute(TaskThread& thread, GraphicPipeline& gpu);
            int getInputPadding(int index = 0) const;
            void getSampledChannels(int index, int& min, int& max) const;

        public:
            static const char* FILTERS_CHUNK_SUFFIX;  //!< suffix added to the op name to get the filters chunk id in the model data
            static const char* BIAS_CHUNK_SUFFIX;     //!< suffix added to the op name to get the bias chunk id in the model data

            /**
                Instantiates a 2D convolution operation.
                \param[in] name                 Operation name
                \param[in] kernelSize           Convolution kernel size
                \param[in] numInputChannels     Number of input feature map channels (input depth)
                \param[in] numOutputChannels    Number of output feature map channels (output depth)
                \param[in] stride               Convolution stride
                \param[in] padding              Padding policy
                \param[in] useBias              If `true`, the bias addition is enabled. The bias vector is searched in the model data.
                \param[in] numGroups            Number of convolution groups to get a group/depthwise convolution
                \param[in] activation           Activation function applied to the operation output
            */
            Conv2D(
                const std::string& name,
                const int kernelSize,
                const int numInputChannels,
                const int numOutputChannels,
                const int stride = 1,
                const Size::Padding padding = Size::Padding::VALID,
                const bool useBias = true,
                const int numGroups = 1,
                const ActivationFunction activation = ActivationFunction::DEFAULT
            );

            inline bool isBiasUsed() const { return useBias; }

            inline int getInputCount()  const { return 2; }
            inline int getOutputCount() const { return 1; }

            inline bool acceptsStorageInput(int index = 0) const { return (index == 0 && !useInputImage) || index == 1; }
            inline bool acceptsStorageOutput(int index = 0) const { return index == 0; }
            inline bool acceptsTextureInput(int index = 0) const { return index == 0 && useInputImage; }

            Size getOutputSize(int outputIndex = 0) const;

            inline Storage::View getOutput(int index = 0) { return output; }

            void setInput(Storage::View&& storage, int inputIndex = 0);
            void setInput(GL::TextureHandler& image, int inputIndex = 0);
            void setOutput(Storage::View&& storage, int outputIndex = 0);

            std::map<std::string, std::string> serialize() const;

            void disconnect();

            /**
                \brief Connects a tensor to a residual input.
                This input is optional. The tensor is added to the convolution result before the non-linear activation
                is applied. Its size must match the output size.
                \param[in] storage      A storage view containing the residual input tensor.
            */
            inline void setResidualInput(Storage::View&& storage) { setInput(std::move(storage), 1); }

            unsigned long countMultiplyAdds() const;
            unsigned long countTexelFetches() const;

            /**
                Sets up deserialization of the operation.
            */
            static bool initDeserializer();
        };

        /**
            \internal
            Being declared here, this variable ensures Conv2D::initDeserializer() is called with inclusion of this header file.
        */
        static const bool CONV2D_OP_DESERIALIZABLE = Conv2D::initDeserializer();
    }
}

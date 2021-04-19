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

namespace Beatmup {
    namespace NNets {

        /**
            2D pooling operation computed on GPU.
            Has a single input and a single output.
            Constraints:
                - Input and output are 3D tensors with values in [0, 1] range sampled over 8 bits.
                - Number of feature maps is a multiple of 4.
                - Pooling area is of square shape.
                - Strides are equal along X and Y.
                - Average pooling only accepts valid zero padding,

            Raspberry Pi-related constraints:
                - Pi cannot sample more than 256 channels to compute a single output value. Actual practical limit is
                  yet lower: pooling size may be limited by 10. When the limit is reached, Pi OpenGL driver reports an
                  out of memory error (0x505).
        */
        class Pooling2D : public AbstractOperation, protected SpatialFilteringMixin {
        public:
            /**
                Pooling operator specification
            */
            enum class Operator {
                MAX,        // max pooling
                AVERAGE     // average pooling
            };

        private:
            const Size size, stride;
            const Operator op;
            const Size::Padding padding;
            Storage::View input, output;
            bool ready;
            GL::RenderingProgram* program;

            void prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank);
            void execute(TaskThread& thread, GraphicPipeline& gpu);
            int getInputPadding(int index = 0) const;
            void getSampledChannels(int index, int& min, int& max) const;

        public:
            /**
                2D pooling layer.
                \param[in] name         Layer name
                \param[in] op           Pooling operator
                \param[in] size         Spatial pooling operational size
                \param[in] stride       Pooling stride; if 0, the size is used
                \param[in] padding      Zero padding applied to the input
            */
            Pooling2D(
                const std::string& name,
                const Operator op,
                const int size,
                const int stride = 1,
                const Size::Padding padding = Size::Padding::VALID
            );

            inline int getInputCount()  const { return 1; }
            inline int getOutputCount() const { return 1; }

            inline bool acceptsStorageInput(int index = 0) const { return index == 0; }
            inline bool acceptsStorageOutput(int index = 0) const { return index == 0; }

            Size getOutputSize(int outputIndex = 0) const;

            inline Storage::View getOutput(int index = 0) { return output; }

            void setInput(Storage::View&& storage, int inputIndex = 0);
            void setOutput(Storage::View&& storage, int outputIndex = 0);

            std::map<std::string, std::string> serialize() const;

            void disconnect();

            unsigned long countTexelFetches() const;

            /**
                Returns a pooling operator from string.
                The conversion is case-insensitive. Raises an exception if cannot interpret the string.
                \param[in] str          The input string
            */
            static Operator operatorFromString(const std::string& str);

            /**
                Sets up deserialization of the operation.
            */
            static bool initDeserializer();
        };

        /**
            \internal
            Being declared here, this variaable ensures Pooling2D::initDeserializer() is called with inclusion of this header file.
        */
        static const bool POOLING2D_OP_DESERIALIZABLE = Pooling2D::initDeserializer();
    }
}

namespace std {
    std::string to_string(Beatmup::NNets::Pooling2D::Operator op);
}
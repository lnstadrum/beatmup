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

#include "../gpu/linear_mapping.h"
#include "operation.h"
#include "storage.h"

namespace Beatmup {
    namespace NNets {

        /**
            Dense (linear) layer.
            Computes `A*x + b` for input feature vector `x`, a matrix `A` and an optional bias vector `b`.
            Accepts a GL::Vector or a flat Storage view on input, amd only a GL::Vector on output.

            Constraints: number of input channels must be a multiple of 8.

            The matrix and bias coefficients are searched in chunks. The chunk names consist of the operation name followed
            by Dense::MATRIX_CHUNK_SUFFIX and Dense::BIAS_CHUNK_SUFFIX respectively.
            The chunk contents is a single precision floating point array.
            The matrix coefficients are taken in row-major order.
        */
        class Dense : public AbstractOperation, private GL::LinearMapping {
        private:
            const int numOutputDims;        //!< output feature vector size / number of rows in `A`
            const bool useBias;             //!< if `true`, the bias vector `b` is searched in the model data to add to the output
            GL::Vector* inputVector;        //!< if not null, points the input vector `x`; otherwise the input is a storage view
            GL::Vector* outputVector;       //!< pointer to the output vector
            Storage::View inputStorage;     //!< if not empty, contains the input features; otherwise the input is a GL vector

            void prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank);
            void execute(TaskThread& thread, GraphicPipeline& gpu);
            void getSampledChannels(int index, int& min, int& max) const;

        public:
            static const char* MATRIX_CHUNK_SUFFIX; //!< suffix added to the op name to get the matrix chunk id in the model data
            static const char* BIAS_CHUNK_SUFFIX;   //!< suffix added to the op name to get the bias chunk id in the model data

            /**
                Creates a Dense operation.
                \param context          A context instance
                \param name             Operation name
                \param numOutputDims    Number of output dimensions
                \param useBias          If `true`, the bias vector addition is enabled.
            */
            Dense(Context& context, const std::string& name, int numOutputDims, bool useBias);

            inline Size getOutputSize(int outputIndex = 0) const {
                return Size(1, numOutputDims, 1);
            }

            inline int getInputCount()  const { return 1; }
            inline int getOutputCount() const { return 1; }

            inline bool acceptsStorageInput(int index = 0) const { return index == 0; }
            inline bool acceptsVectorInput(int index = 0) const { return index == 0; }
            inline bool acceptsVectorOutput(int index = 0) const { return index == 0; }

            void getOutput(GL::Vector*&, int index = 0);

            void setInput(Storage::View&& view, int index = 0);
            void setInput(GL::Vector& vector, int index = 0);
            void setOutput(GL::Vector& vector, int index = 0);

            /**
                \return a supported data format for GL::Vector on output.
            */
            GL::Vector::Format getOutputVectorFormat() const;

            std::map<std::string, std::string> serialize() const;
            void disconnect();

            inline unsigned long countMultiplyAdds() const {
                const int numInputDims = inputVector ? inputVector->getSize() : inputStorage.getSize().volume();
                return numInputDims * numOutputDims;
            }

            /**
                Sets up deserialization of the operation.
            */
            static bool initDeserializer();
        };

        /**
            \internal
            Being declared here, this variable ensures Dense::initDeserializer() is called with inclusion of this header file.
        */
        static const bool DENSE_OP_DESERIALIZABLE = Dense::initDeserializer();
    }
}

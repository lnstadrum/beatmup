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
#include <vector>
#include "operation.h"
#include "../gpu/linear_mapping.h"


namespace Beatmup {
    namespace NNets {
        /**
            Softmax layer.
            It does not have output, but acts as a sink. The resulting probabilities are returned by getProbabilities().
            This operation is executed on CPU.
        */
        class Softmax : public CpuOperation {
        private:
            std::vector<float> output;
            std::vector<float> partialSums;
            Storage::View inputView;
            GL::Vector* inputVector;

            int getAmountOfWork() const;
            void beforeExecute(GraphicPipeline& gpu, const int threadCount);
            void execute(const int sliceStart, const int sliceStop, const int threadIdx, const int threadCount);
            void afterExecute(const int threadCount);

        public:
            /**
                Creates a softmax layer.
                \param[in] name     Operation name
            */
            Softmax(const std::string& name = "Softmax");

            inline int getInputCount()  const { return 1; }
            inline int getOutputCount() const { return 0; }

            inline bool acceptsStorageInput(int index = 0) const { return index == 0; }
            inline bool acceptsVectorInput(int index = 0) const { return index == 0; }

            void setInput(Storage::View&& view, int index = 0);
            void setInput(GL::Vector& vector, int index = 0);

            std::map<std::string, std::string> serialize() const;

            void disconnect();

            const std::vector<float>& getProbabilities() const { return output; }

            /**
                Sets up deserialization of the operation.
            */
            static bool initDeserializer();
        };

        /**
            \internal
            Being declared here, this variaable ensures Softmax::initDeserializer() is called with inclusion of this header file.
        */
        static bool SOFTMAX_OP_DESERIALIZABLE = Softmax::initDeserializer();
    }
}

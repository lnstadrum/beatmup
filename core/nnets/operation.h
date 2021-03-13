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
#include "../context.h"
#include "storage.h"
#include "../gpu/linear_mapping.h"
#include "../gpu/program.h"
#include "../gpu/program_bank.h"
#include "../utils/chunkfile.h"
#include "../utils/progress_tracking.h"
#include "../utils/string_builder.h"
#include "../utils/listing.h"
#include <string>

namespace Beatmup {
    namespace NNets {
        class Model;

        /**
            Abstract neural net operation (layer).
            Has a name used to refer the operation in a Model. The operation data (such as convolution weights) is provided through a ChunkCollection
            in single precision floating point format, where the chunks are searched by operation name.
            Operations have several inputs and outputs numbered starting from zero. Different operations accept different kinds of input and output
            data.
             - Inputs may be Storage, GL::TextureHandler or GL::Vector.
             - Outputs may be Storage or GL::Vector.
            The operations are built in a deferred fashion during the first inference run, which makes the first run much slower than the subsequent
            runs.
        */
        class AbstractOperation {
            friend class Model;
            AbstractOperation(const AbstractOperation&) = delete;    //!< disabling copying constructor
        private:
            std::string name;

        protected:
            AbstractOperation(const std::string& name): name(name) {}

            /**
                Compiles GLSL shaders.
                \param[in,out] gpu      A graphic pipeline instance
                \param[in,out] data     Chunkfile containing operation data (e.g. weights and biases)
                \param[in,out] bank     A program bank with existing GLSL programs to be reused when possible.
                                        If a new program is built, it is added to the bank.
            */
            virtual void prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank) = 0;

            /**
                Executes the operation.
                The operation should be prepared.
                \param[in,out] thread       Calling CPU thread descriptor
                \param[in,out] gpu          A graphic pipeline instance
            */
            virtual void execute(TaskThread& thread, GraphicPipeline& gpu) = 0;

            /**
                Executes the operation within a specific CPU thread.
                \param[in,out] thread       Calling CPU thread descriptor
            */
            virtual void execute(TaskThread& thread) {}

            /**
                Retrieves minimum required size of zero padding for a given input.
                Operations that sample a neighborhood of a pixel may need the input to be padded with zeros, if some of the neighboring samples fall
                out of the are containing data. In %Beatmup the zero padding is handled by allocating a bigger input and putting zeros around the area
                that is actually filled with data.
                \return number of zero columns and rows to be added to the input area.
            */
            virtual int getInputPadding(int index = 0) const { return 0; }

            /**
                Retrieves range of input features channels sampled at the same time for a specific input.
                The operation would typically take the entire storage and sample it at once, if needed. If the number of textures in a storage
                exceeds the number of texture samplers that the GPU may use simultaneously, an exception occurs. This function provides the necessary
                information to limit the number of textures in the storage when allocating it. When the limit is reached, multiple channels are
                packed into a single texture in the storage.
                \param[in] index        The input index. Expected to fall in the valid range, i.e. from zero to getInputCount() - 1 inclusive.
                \param[out] min         The minimum number of channels that can be sampled at once
                \param[out] max         The maximum number of channels that can be sampled at once
            */
            virtual void getSampledChannels(int index, int& min, int& max) const = 0;

        public:
            class InconsistentModelData : public Exception {
            public:
                InconsistentModelData(const AbstractOperation* op, const char* shortDescription) :
                    Exception("Error when configuring '%s' operation: %s.", op->getName().c_str(), shortDescription)
                {}

                InconsistentModelData(const AbstractOperation* op, const char* shortDescription, const size_t entriesRequested, const size_t sizeBytesGot) :
                    Exception("Error when configuring '%s' operation: %s. Cannot convert %u bytes of the given weights to %u requested entries.",
                        op->getName().c_str(), shortDescription, (unsigned int)sizeBytesGot, (unsigned int)entriesRequested)
                {}
            };

            class NotReady : public Exception {
            public:
                NotReady(const AbstractOperation* op): Exception("Operation not ready: '%s'", op->getName().c_str()) {}
            };

            virtual ~AbstractOperation() {}

            /**
                Returns `true` if the operation is run on GPU.
                Otherwise, it is run on CPU.
            */
            virtual bool usesGpu() const { return true; }

            /**
                Returns number of operation inputs.
                Inputs are then indexed from zero to the returned value minus one inclusive.
            */
            virtual int getInputCount()  const { return 0; }

            /**
                Returns number of operation outputs.
                Outputs are then indexed from zero to the returned value minus one inclusive.
            */
            virtual int getOutputCount() const { return 0; }

            /**
                Returns `true` if the operation can take a Storage::View at a specific input.
                Neural network operations may accept different kinds of data containers on inputs and outputs, namely Storage::View, GL::Vector
                and textures. This function is used to check whether a given operation accepts a storage view on input.
                \param[in] index        The input index. Expected to fall in the valid range, i.e. from zero to getInputCount() - 1 inclusive.
            */
            virtual bool acceptsStorageInput(int index = 0) const { return false; }

            /**
                Returns `true` if the operation can take a GL::Vector at a specific input.
                Neural network operations may accept different kinds of data containers on inputs and outputs, namely Storage::View, GL::Vector
                and textures. This function is used to check whether a given operation accepts a vector on input.
                \param[in] index        The input index. Expected to fall in the valid range, i.e. from zero to getInputCount() - 1 inclusive.
            */
            virtual bool acceptsVectorInput(int index = 0) const { return false; }

            /**
                Returns `true` if the operation can take a GL::TextureHandler at a specific input.
                Neural network operations may accept different kinds of data containers on inputs and outputs, namely Storage::View, GL::Vector
                and textures. This function is used to check whether a given operation accepts a texture on input.
                \param[in] index        The input index. Expected to fall in the valid range, i.e. from zero to getInputCount() - 1 inclusive.
            */
            virtual bool acceptsTextureInput(int index = 0) const { return false; }

            /**
                Returns `true` if the operation can take a Storage::View at a specific output.
                Neural network operations may accept different kinds of data containers on outputs and outputs, namely Storage::View, GL::Vector
                and textures. This function is used to check whether a given operation accepts a storage view on output.
                \param[in] index        The output index. Expected to fall in the valid range, i.e. from zero to getOutputCount() - 1 inclusive.
            */
            virtual bool acceptsStorageOutput(int index = 0) const { return false; }

            /**
                Returns `true` if the operation can take a GL::Vector at a specific output.
                Neural network operations may accept different kinds of data containers on outputs and outputs, namely Storage::View, GL::Vector
                and textures. This function is used to check whether a given operation accepts a vector on output.
                \param[in] index        The output index. Expected to fall in the valid range, i.e. from zero to getOutputCount() - 1 inclusive.
            */
            virtual bool acceptsVectorOutput(int index = 0) const { return false; }

            /**
                Returns `true` if the operation can take a GL::TextureHandler at a specific output.
                Neural network operations may accept different kinds of data containers on outputs and outputs, namely Storage::View, GL::Vector
                and textures. This function is used to check whether a given operation accepts a texture on output.
                \param[in] index        The output index. Expected to fall in the valid range, i.e. from zero to getOutputCount() - 1 inclusive.
            */
            virtual bool acceptsTextureOutput(int index = 0) const { return false; }

            /**
                Returns full size of a specific operation output.
            */
            virtual Size getOutputSize(int outputIndex = 0) const;

            /**
                Returns a storage view bound to a specific operation output.
                If no view is bound, returns empty view.
                \param[in] index        The output index. Expected to fall in the valid range, i.e. from zero to getOutputCount() - 1 inclusive.
            */
            virtual Storage::View getOutput(int index = 0);

            /**
                Returns a GL::Vector bound to a specific operation output.
                \param[out] vector      Pointer to the GL::Vector. If no vector is bound, becomes null.
                \param[in] index        The output index. Expected to fall in the valid range, i.e. from zero to getOutputCount() - 1 inclusive.
            */
            virtual void getOutput(GL::Vector*& vector, int index = 0);

            /**
                Returns a GL::TextureHandler bound to a specific operation output.
                \param[out] vector      Pointer to the GL::TextureHandler. If no texture is bound, becomes null.
                \param[in] index        The output index. Expected to fall in the valid range, i.e. from zero to getOutputCount() - 1 inclusive.
            */
            virtual void getOutput(GL::TextureHandler*& vector, int index = 0);

            virtual void setInput(Storage::View&& storage, int index = 0);
            virtual void setOutput(Storage::View&& storage, int index = 0);
            virtual void setInput(GL::Vector& vector, int index = 0);
            virtual void setOutput(GL::Vector& vector, int index = 0);
            virtual void setInput(GL::TextureHandler& image, int index = 0);
            virtual void setOutput(GL::TextureHandler& image, int index = 0);

            /**
                Returns a serialized representation of th operation;
            */
            virtual std::map<std::string, std::string> serialize() const = 0;

            /**
                Assigns empty inputs and outputs
            */
            virtual void disconnect() = 0;

            /**
                Counts (approximate) number of multiply-adds used by this operation.
                A single multiply-add is one multiplication and one addition.
            */
            virtual inline unsigned long countMultiplyAdds() const { return 0; }

            /**
                \return operation name
            */
            std::string getName() const { return name; }

            /**
                Enables construction of an operation from its serialized representation.
                Every new instance of this class is registered in a static map and used to build operations during the model deserialization.
            */
            class Deserializer {
            public:
                static std::map<std::string, Deserializer*>& getDeserializersMap();

                /**
                    Registers a new deserializer capable of building an operation from a listing block.
                    \param[in] opType       Operation type
                */
                Deserializer(const char* opType);

                /**
                    Constructs an operation from a serialized representation (a listing block).
                    Raises exceptions if the representation contains errors or has missing parameters.
                    \param[in] context      A context instance passed to the operation constructor if needed
                    \param[in] block        The block containing the operation description
                */
                virtual AbstractOperation* deserialize(Context& context, const Listing::Block& block) = 0;
            };
        };


        /**
            Generates GLSL fragment shader code sampling a local neighborhood around the current texture coordinates for further filtering.
        */
        class SpatialFilteringMixin {
        private:
            const int nbSizeX, nbSizeY;
            Point shift;            //!< current static shift of the sampling position
            float* deltas;          //!< array storing pixel position differences for neighborhood sampling
            bool useUniformShift;   //!< if `true`, the sampling position can be shifted dynamically at every run

            /**
                \return number of 2D delta-vectors stored in a uniform variable in the fragment shader
            */
            int getDeltasSize() const { return std::max(1, std::max(nbSizeX, nbSizeY) / 2); }

        protected:
            static const char* SAMPLE_ID_PREFIX;    //!< prefix of variables declaring a neighbor sample

            /**
                Initializes spatial filtering mixin
                \param[in] nbSizeX    Neighborhood width in samples
                \param[in] nbSizeY    Neighborhood height in samples
            */
            SpatialFilteringMixin(const int nbSizeX, const int nbSizeY);

            ~SpatialFilteringMixin();

            /**
                Writes out the very GLSL fragment shader header required for spatial neighborhood sampling
                \param[in,out] code             The GLSL code storage
                \param[in] useUniformShift      If `true`, the sampling position can be shifted dynamically at every run
            */
            void writeHeader(StringBuilder& code, bool useUniformShift);

            /**
                Declares GLSL fragment shader main(..) code part required for spatial neighborhood sampling
                \param[in,out] code         The GLSL code storage
                \param[in] datatype         The neighborhood samples datatype
                \param[in] inlineSampling   If `true`, sampled values are not stored in intermediate variables
                                            (sampleInline() can only be used, not sample())
            */
            void declare(StringBuilder& code, const char* datatype, bool inlineSampling = false);

            /**
                Samples a neighborhood of a given texture.
                \param[in,out] code         The GLSL code storage
                \param[in] inputName        The texture sampler array variable name
                \param[in] inputIndex       Index of the texture in the array
                \param[in] shift            Sampling position shift
                \param[in] isFirstSample    If `true`, this is a very first sampling operation. A required initial setup will be run.
                \param[in] suffix           Sampling operation suffix, e.g. ".rgb" to sample 3 values only
            */
            void sample(StringBuilder& code, const char* inputName, const int inputIndex, const Point& shift, const bool isFirstSample = true, const char* suffix = "");

            void sampleInline(StringBuilder& code, const char* inputName, const int inputIndex, const IntPoint& position, const Point& shift, const char* suffix = "");

            /**
                Prepares the spatial filtering operation execution.
                \param[in] width    Input texture width in pixels
                \param[in] height   Input texture height in pixels
            */
            void setup(const int width, const int height);

            /**
                Applies an offset to the sampling position at runtime. Only used if the uniform shift is enabled when writing out the header code.
                \param[in,out] program    The program
                \param[in] shift        The shift in pixels
                \param[in] inputSize    Size of the sampled input texture in pixels
            */
            void setUniformShift(GL::Program& program, const IntPoint& shift, const IntPoint& inputSize);

            /**
                Prepares a given program for spatial filtering.
                \param[in,out] program    The program
            */
            void setupProgram(GL::Program& program);

            /**
                Implements common padding policies by computing a rectangular area of positions the sampling kernel takes in order to get the result
                with the required padding.
                \param[in] size         The input size in pixels
                \param[in] stride       The stride
                \param[in] padding      The padding policy
                \return kernel center point positions in pixels.
            */
            IntRectangle getSamplingArea(const IntPoint& size, const IntPoint& stride, const Size::Padding padding)  const;

            /**
                Computes area in pixels to sample a given storage according to specific stride and padding.
                \param[in] storage      The input storage to sample
                \param[in] channel      Number of the channel to sample
                \param[in] stride       The stride
                \param[in] padding      The padding policy
                \return kernel center point positions in pixels.
            */
            IntRectangle getSamplingArea(
                const Storage::View& storage,
                const int channel,
                const IntPoint& stride,
                const Size::Padding padding
            ) const;

            /**
                Computes texture coordinates sampling a specific storage channel for given stride, padding and output size.
                \param[in] storage      The storage to sample
                \param[in] channel      The channel number in the storage to sample
                \param[in] stride       The stride
                \param[in] padding      The padding
                \param[in] outputSize   Size of output texture in pixels
                \return rectangle containing texture coordinates.
            */
            Rectangle getTextureCoordinates(
                const Storage::View& storage,
                const int channel,
                const IntPoint& stride,
                const Size::Padding padding,
                const IntPoint& outputSize
            ) const;

            /**
                Retrieves input sampling point position for the current fragment. Defined after declare() is called.
                \return the GLSL expression of the sampling position.
            */
            std::string getInputSamplingPos() const;

            inline bool isUniformShiftUsed() const { return useUniformShift; }
        };


        /**
            Activation function specification
        */
        enum class ActivationFunction {
            DEFAULT,        //!< default activation: 0..1 bounded ReLU (identity clipped to 0..1 range)
            BRELU6,         //!< 0.167 times identity clipped to 0..1 range
            SIGMOID_LIKE    //!< piecewise-linear sigmoid approximation
        };


        /**
            A mixin implementing activation functions in GLSL.
            The result of any operation must fit into 0..1 range to be stored as a color texture. The activation
            function introduces non-linear behavior in the model and determines how the computed values are mapped to
            the valid output range.
        */
        class ActivationFunctionMixin {
        private:
        protected:
            const ActivationFunction activationFunc;

            inline ActivationFunctionMixin(const ActivationFunction activationFunc): activationFunc(activationFunc) {}

            /**
                Renders a GLSL code applying activation function to a specific variable and writing the result to
                gl_FragColor shader output variable.
                \param[in,out] code             A GLSL source code to be appended by the activation function code
                \param[in] inputVariable        Name of the variable to apply the activation function to
            */
            void apply(StringBuilder& code, const char* inputVariable);
        };


        /**
            Operation computed on CPU
        */
        class CpuOperation : public AbstractOperation {
        protected:
            CpuOperation(const std::string& name) : AbstractOperation(name) {}

            inline void prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank) {}

            inline void getSampledChannels(int index, int& min, int& max) const { min = max = 0; }

            /**
                Returns amount of work in arbitrary units to be splitted among threads.
            */
            virtual int getAmountOfWork() const = 0;

            /**
                Called right before the operation is executed
            */
            virtual void beforeExecute(GraphicPipeline& gpu, const int threadCount) {}

            /**
                Called right after the operation is executed
            */
            virtual void afterExecute(const int threadCount) {}

            void execute(TaskThread& thread, GraphicPipeline& gpu);
            void execute(TaskThread& thread);

            /**
                Executes the operation body within a specific CPU thread.
                The threads can process different slices according to a given amount of work (see getAmountOfWork()).
                \param[in] sliceStart       Current slice starting point (included in the slice)
                \param[in] sliceStop        Current slice end point (excluded from the slice)
                \param[in] threadIdx        Zero-based calling thread number
                \param[in] threadCount      Total number of threads executing the operation
            */
            virtual void execute(const int sliceStart, const int sliceStop, const int threadIdx, const int threadCount) = 0;
        public:
            bool usesGpu() const { return false; }
        };


        /**
            Returns a zero padding value from a string.
            The conversion is case-insensitive. Raises an exception if cannot interpret the string.
            \param[in] str          The input string
        */
        ActivationFunction activationFunctionFromString(const std::string& str);
    }
}

namespace std {
    std::string to_string(Beatmup::NNets::ActivationFunction function);
}
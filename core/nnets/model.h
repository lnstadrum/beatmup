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
#include "storage.h"
#include "../bitmap/internal_bitmap.h"
#include "../gpu/program_bank.h"
#include "../gpu/linear_mapping.h"
#include "../context.h"
#include "../utils/progress_tracking.h"
#include "../utils/profiler.h"
#include "../utils/chunkfile.h"
#include "../utils/listing.h"
#include <vector>
#include <initializer_list>


namespace Beatmup {
    /**
        \page NNetsModuleOverview NNets module overview
        %Beatmup provides a way to run inference of user-defined neural networks on GPU using OpenGL.

        The neural network (a NNets::Model instance) can be built in one of two ways:
         - layer-by-layer in the user code, by adding instances of NNets::AbstractOperation,
         - loading a model using NNets::DeserializedModel from a Yaml-like text description (see \subpage NNetsModelSerialization).

        The model data (e.g., convolution filters values) is stored in a ChunkCollection as plain single precision floating point arrays. They are
        indexed using the operation names.
        The model instance and input/output containers are supplied to NNets::InferenceTask which can be run in a thread pool of a Context, just
        as another AbstractTask.

        Under the hood, the network is converted into a set of OpenGL ES 2.0-compliant GLSL shaders. The data is stored in textures in GPU memory.
        %Beatmup takes care of building and executing shader programs.

        With this %Beatmup enables hardware-accelerated inference on any decent GPU, keeping the CPU available for other tasks. It allows to deploy
        easily the same model on various hardware, including inexpensive single-board computers, %Android GPUs, integrated and discrete desktop GPUs
        from any vendor.

        However, NNets module is still quite young and comes with a set of constraints.
         - The set of implemented features is limited. So far it is oriented to image classification and feature extraction exclusively. See
           NNets::AbstractOperation subclasses for the list of implemented neural network operations.
         - Not any model can be transformed into a %Beatmup-compliant model. Most likely, a model needs to be designed and trained from scratch to be
           deployed with %Beatmup. See NNets::Conv2D, NNets::Pooling2D and NNets::Dense operations descriptions for their corresponding constraints.
         - OpenGL may introduce a significant overhead. The inference thoughput achievable with %Beatmup on powerful desktop GPUs is much likely
           limited compared to what can be achieved with vendor-specific proprietary technologies widely used for training and inference.
         - There are constraints related to the OpenGL ES 2.0 backend.
           - The activations of almost all operations are stored as 8-bit integers. This may require the training to be somehow aware of the
             activations quantization, otherwise with the increasing depth the error due to the quantization may cause performance degradation.
             However, the weights of the network are usually not quantized:
             - Conv2D filters and biases are stored in a floating point format. Possible quantization may apply if a given GPU does not support the
               single precision floating point computations.
             - Dense layers matrices and bias vectors are stored in floating point format if the GPU is OpenGL ES 3.1-compliant. Otherwise, a 16 bits
               fixed point representation is used.
           - The 8-bit sampled activations cover [0, 1] range. This strongly limits the activation functions that can be used in the model.
           - OpenGL may be inefficient to sample many feature channels at the same time or have hardware or driver-defined hard limit on the number
             of samples per output value (the latter is the case for Raspberry Pi). This constraints the width of the network. To overcome this,
             group convolutions and \ref NNetsShufflingExplained "channel shuffling" are suggested. The latter allows to shuffle channels between
             layers literally for free, which helps to increase the connectivity across the width of the network for group convolutions in
             particular.
         - The batch size is fundamentally and unconditionally equal to 1, i.e., the inference is run for one given input image at a time.
    */

    /**
        Neural nets inference on GPU using OpenGL.
    */
    namespace NNets {

        /**
            Neural net model.
            Contains a list of operations and programmatically defined interconnections between them using addConnection().
            Enables access to the model memory at any point in the model through addOutput() and getOutputData().
            The memory needed to store internal data during the inference is allocated automatically; storages are reused when possible.
            The inference of a Model is performed by InferenceTask.
        */
        class Model : public GL::ProgramBank {
        private:
            /**
                Connection descriptor.
                For a given source operation describes a connection with another operation.
            */
            typedef struct {
                AbstractOperation* dest;    //!< destination operation
                int output;                 //!< output index
                int input;                  //!< input index
                int shuffle;                //!< shuffling step (details \ref NNetsShufflingExplained "here")
            } Connection;

            /**
                A user-defined output descriptor.
            */
            typedef struct {
                int index;                  //!< operation output index to fetch data from
                std::vector<float> data;    //!< container to store the data
            } UserOutput;

            std::multimap<const AbstractOperation*, Connection> connections;        //!< source operation => connection descriptor mapping
            std::multimap<const AbstractOperation*, UserOutput> userOutputs;        //!< operation => user output mapping

            std::vector<Storage*> storages;         //!< allocated storages used during the inference
            std::vector<GL::Vector*> vectors;       //!< allocated vectors used during the inference
            std::vector<InternalBitmap*> textures;  //!< allocated images used during the inference
            Profiler* profiler;                     //!< pointer to a Profiler attached to the model

        protected:
            std::vector<AbstractOperation*> ops;    //!< model operations
            ProgressTracking preparingProgress;     //!< model preparation progress
            ProgressTracking inferenceProgress;     //!< inference progress
            bool ready;                             //!< if `true`, ops are connected to each other and storages are allocated

            /**
                Frees all allocated storages.
            */
            void freeMemory();

            /**
                Allocates a new storage. Its views might be used as operations inputs and outputs.
                The storage is destroyed together with the model.
                \param[in,out] gpu              A graphic pipeline instance
                \param[in] size                 The storage size (width, height, number of channels)
                \param[in] forGpu               Allocate for the use on GPU
                \param[in] forCpu               Allocate for the use on CPU
                \param[in] pad                  Storage padding: number of pixels added on both sides along width and height of every channel
                \param[in] reservedChannels     Number of additional channels that may be sampled together with the storage.
                                                This does not change the storage size, but impacts the way the channels are packed into the textures.
                                                It allows the storage to be sampled with other storages of a specific total depth in the same shader,
                                                if the addDepth is greater or equal to the total depth.
                \return newly allocated storage.
            */
            Storage& allocateStorage(GraphicPipeline& gpu, const Size size, bool forGpu = true, bool forCpu = false, const int pad = 0, const int reservedChannels = 0);

            /**
                Allocates a new flat storage. Its views are be used as operations inputs and outputs.
                Flat storages can be inputs of Dense layers.
                The storage is destroyed together with the model.
                \param[in,out] gpu          A graphic pipeline instance
                \param[in] size             Number of samples in the storage
                \return newly allocated storage.
            */
            Storage& allocateFlatStorage(GraphicPipeline& gpu, const int size);

            /**
                Allocates a vector that can be used as operation input or output.
                Differently to flat storages, vectors store floating point data (GL ES 3.1 and higher) or 16-bit signed fixed point values with 8 bits
                fractional part (GL ES 2.0).
                \param[in,out] gpu          A graphic pipeline instance
                \param[in] size             Number of samples in the vector
            */
            GL::Vector& allocateVector(GraphicPipeline& gpu, const int size);

            /**
                Allocates a texture that can be used as operation input or output.
                \param[in,out] gpu          A graphic pipeline instance
                \param[in] size             Image size. The depth can be 1, 3 or 4 channels.
            */
            InternalBitmap& allocateTexture(GraphicPipeline& gpu, const Size size);

            /**
                Checks whether an operation goes before another operation in the model according the ops execution order.
                \param[in] first        The first operation (expected to be executed earlier)
                \param[in] second       The first operation (expected to be executed later)
                \return `true` if both operations are in the model, and the first one is executed before the second one, `false` otherwise.
            */
            bool isPreceding(const AbstractOperation& first, const AbstractOperation& second) const;


            AbstractOperation* operator[](const std::string& operationName);
            const  AbstractOperation* operator[](const std::string& operationName) const;

            void addConnection(AbstractOperation& source, AbstractOperation& dest, int output = 0, int input = 0, int shuffle = 0);

        public:
            /**
                Instantiates a model from a list of operations interconnecting them in a feedforward fashion.
                The first output of every operation is connected to the first input of its successor.
                Optional connections may be added after model creation.
                \param[in,out] context          A context instance
                \param[in] ops                  Operations given in the execution order. The Model does not take ownership of them.
            */
            Model(Context& context, std::initializer_list<AbstractOperation*> ops);

            /**
                Instantiates an empty model.
                \param[in,out] context          A context instance used to store internal resources needed for inference
            */
            Model(Context& context);
            ~Model();

            /**
                Adds a new operation to the model.
                The operation is added to the end of the operations list. The execution order corresponds to the addition order.
                The Model does not takes ownership of the passed pointer.
                \param[in] newOp                The new operation
                \param[in] connect              If `true`, the main operation input (#0) is connected to the main output (#0) of the last operation
            */
            void append(AbstractOperation* newOp, bool connect = false);

            /**
                Adds new operations to the model.
                The operations are added to the end of the operations list. The execution order corresponds to the addition order.
                The Model does not takes ownership of the passed pointer.
                \param[in] newOps               The new operations
                \param[in] connect              If `true`, the main input (#0) of every operation is connected to the main output (#0)
                                                of the preceding operation
            */
            void append(std::initializer_list<AbstractOperation*> newOps, bool connect = false);

            /**
                Adds a new operation to the model before another operation in the execution order.
                The Model does not takes ownership of the passed pointer. The new operation is not automatically connected to other operations.
                \param[in] opName               Name of the operation the new operation is inserted before
                \param[in] newOp                The new operation
            */
            void addOperation(const std::string& opName, AbstractOperation* newOp);
            void addOperation(const AbstractOperation& operation, AbstractOperation* newOp);

            /**
                Adds a connection between two given ops.
                \param[in] sourceOpName         Name of the operation emitting the data
                \param[in] destOpName           Name of the operation receiving the data
                \param[in] output               Output number of the source operation
                \param[in] input                Input number of the destination operation
                \param[in] shuffle              If greater than zero, the storage is shuffled.
                                                For shuffle = `n`, the output channels are sent to the destination operation in the following order:
                                                  0, 1, 2, 3, 4n, 4n+1, 4n+2, 4n+3, 8n, 8n+1, 8n+2, 8n+3, ..., 4, 5, 6, 7, 4n+4, 4n+5, 4n+6, 4n+7, 8n+4, ...
                \anchor NNetsShufflingExplained
            */
            void addConnection(const std::string& sourceOpName, const std::string& destOpName, int output = 0, int input = 0, int shuffle = 0);

            /**
                Enables reading output data from the model memory through getOutputData().
                A given operation output is connected to a storage that might be accessed by the application after the run.
                \param[in] operation            Name of the operation or the operation itself to get data from
                \param[in] output               The operation output index
            */
            void addOutput(const std::string& operation, int output = 0);
            void addOutput(const AbstractOperation& operation, int output = 0);

            /**
                Reads data from the model memory.
                addOutput() is needed to be called first in order to enable reading the data. Otherwise null is returned.
                \param[out] numSamples          Returns number of samples in the pointed data buffer
                \param[in] operation            Name of the operation or the operation itself to get data from
                \param[in] output               The operation output index
                \return pointer to the data stored as a 3D array of (height, width, channels) layout, or null.
            */
            const float* getOutputData(size_t& numSamples, const std::string& operation, int output = 0) const;
            const float* getOutputData(size_t& numSamples, const AbstractOperation& operation, int output = 0) const;

            /**
                Prepares all operations: reads the model data from chunks and builds GPU programs.
                The inputs of the model needed to be provided.
                Preparation progress is tracked by a ProgressTracking instance (getPreparingProgress()).
                \param[in,out] gpu              A graphic pipeline instance
                \param[in] data                 ChunkCollection containing the model data
            */
            virtual void prepare(GraphicPipeline& gpu, ChunkCollection& data);

            /**
                \return `true` if the model is ready to be used for inference (prepare() has been called).
            */
            inline bool isReady() const { return ready; }

            /**
                Runs the inference.
                \param[in,out] thread       Task thread instance
                \param[in,out] gpu          A graphic pipeline
            */
            void execute(TaskThread& thread, GraphicPipeline* gpu);

            /**
                Checks if a specific operation makes part of the model.
                \return `true` if the operation is in the model.
            */
            bool isOperationInModel(const AbstractOperation& operation) const;

            inline AbstractOperation& getFirstOperation() { return *ops.front(); }
            inline AbstractOperation& getLastOperation () { return *ops.back(); }
            inline const AbstractOperation& getFirstOperation() const { return *ops.front(); }
            inline const AbstractOperation& getLastOperation () const { return *ops.back(); }
            inline size_t getNumberOfOperations() const { return ops.size(); }

            /**
                Retrieves an operation by its name
            */
            template<class OperationClass = AbstractOperation>
            inline OperationClass& getOperation(const std::string& operationName) {
                return *static_cast<OperationClass*>((*this)[operationName]);
            }

            /**
                Returns model preparation progress tracking.
            */
            inline const ProgressTracking& getPreparingProgress() const { return preparingProgress; }

            /**
                Returns inference progress tracking.
            */
            inline const ProgressTracking& getInferenceProgress() const { return inferenceProgress; }

            /**
                Provides an estimation of the number of multiply-adds characterizing the model complexity.
                Queries the number of multiply-adds of every operation of the model and sums them up.
            */
            unsigned long countMultiplyAdds() const;

            /**
                Returns the amount of texture memory in bytes currently allocated by the model to run the inference.
                When the model is ready to run, this represents the size of the memory needed to store internal data during the inference.
                The resulting value does not include the size of GLSL shaders binaries stored in GPU memory which can be significant.
            */
            size_t getMemorySize() const;

            /**
                Returns serialized representation of the model as a Listing.
            */
            Listing serialize() const;

            /**
                Returns serialized representation of the model as a string.
            */
            std::string serializeToString() const;

            /**
                Attaches a profiler instance to meter the execution time per operation during the inference.
                This may slow down the inference.
                \param[in] profiler     A profiler instance or null pointer (to disable the profiling)
            */
            inline void setProfiler(Profiler* profiler) { this->profiler = profiler; }
        };


        /**
            Wrapper for exceptions occuring during the model inference
        */
        class InferenceTimeError : public Exception {
        public:
            InferenceTimeError(const AbstractOperation& op, const std::exception& ex);
        };
    }
}

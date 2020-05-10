/**
    Neural nets operations
 */
#pragma once
#include "../storage.h"
#include "../../../utils/chunkfile.h"
#include "../../../utils/progress_tracking.h"
#include <string>

namespace Beatmup {
    namespace NNets {

        class AbstractOperation {
            friend class Model;
            friend class Inference;
            AbstractOperation(const AbstractOperation&) = delete;	//!< disabling copying constructor
        private:
            std::string name;
        protected:
            virtual ~AbstractOperation();
            std::string getProgramChunkId(size_t idx) const;
            /**
                Stores the operation in its prepared state to cache.
            */
            virtual void store(GraphicPipeline& gpu, ChunkFile::Writer& cache, ProgressTracking& progress);
            /**
                Loads an prepared operation from cache.
            */
            virtual void load(GraphicPipeline& gpu, ChunkFile& cache, ProgressTracking& progress);
            /**
                Performs the operation
            */
            virtual void perform(GraphicPipeline& gpu) = 0;
        public:
            class InconsistentModelData : public Exception {
            public:
                InconsistentModelData(const AbstractOperation* op, const char* shortDescription) :
                    Exception("Error when configuring '%s' operaion: %s.", op->getName().c_str(), shortDescription)
                {}
                InconsistentModelData(const AbstractOperation* op, const char* shortDescription, const size_t entriesRequested, const size_t sizeBytesGot) :
                    Exception("Error when configuring '%s' operaion: %s. Cannot convert %u bytes of the given weights to %u requested entries.",
                        op->getName().c_str(), shortDescription, (unsigned int)sizeBytesGot, (unsigned int)entriesRequested)
                {}
            };
            class NotReady : public Exception {
            public:
                NotReady(const AbstractOperation* op) :
                    Exception("Operation not ready: '%s'", op->getName().c_str())
                {}
            };
            class TypeMismatch : public Exception {
            public:
                TypeMismatch(Storage::Type expected, Storage::Type got) :
                    Exception("Storage type mismatch: %s got while %s expected.", Storage::toString(got).c_str(), Storage::toString(expected).c_str())
                {}
                static void check(Storage::Type expected, Storage::Type got);
            };
            AbstractOperation(const std::string& name);
            virtual void prepare(GraphicPipeline& gpu, ChunkFile& data, ProgressTracking& progress) = 0;
            virtual int getInputCount() const;
            virtual int getOutputCount() const;
            virtual Size getInputSize(int inputIndex = 0) const = 0;
            virtual Size getOutputSize(int outputIndex = 0) const = 0;
            virtual Storage::Type getInputType(int inputIndex = 0) const;
            virtual Storage::Type getOutputType(int outputIndex = 0) const;
            /**
                Checks whether a suggested storage type is accepted on a specified operation input
                \param[in] type			The suggested storage type
                \param[in] inputIndex	The input index
                \return `true` if the suggested type may be used on the given operation input, `false` otherwise.
            */
            virtual bool setInputType(Storage::Type type, int inputIndex = 0);
            virtual void setInput(Storage& storage, int inputIndex = 0) = 0;
            virtual void setOutput(Storage& storage, int outputIndex = 0) = 0;
            /**
                \brief Allocates a storage for a specified operation output.
                The storage is further handled on the calling side.
                The allocated storage is not bound to the output. setOutput() has to be called.
                \param[in] outputIndex		The output index
                \return appropriate texture handler capable of storing the operation output.
            */
            virtual Storage* allocateOutput(int outputIndex = 0) const = 0;
            /**
                \return Operation name
            */
            std::string getName() const { return name; }
            /**
                Gives number of calls ProgressTracking is called during dedicated processing stages.
            */
            virtual int getMaxProgress() const { return 1; }
        };

        /**
            An operation running on GPU using one or several compute programs
        */
        class GPUOperation : public AbstractOperation {
        private:
            std::vector<GL::Object<GL::ComputeProgram>*> programs;
            void perform(GraphicPipeline& gpu);
        protected:
            Context& ctx;
            ~GPUOperation();
            void store(GraphicPipeline& gpu, ChunkFile::Writer& cache, ProgressTracking& progress);
            void load(GraphicPipeline& gpu, ChunkFile& cache, ProgressTracking& progress);
            virtual std::vector<std::string> generateCode(GraphicPipeline& gpu, ChunkFile& data) = 0;
            virtual void perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int workerIdx) const = 0;
        public:
            GPUOperation(Context& ctx, const std::string& name);
            void prepare(GraphicPipeline& gpu, ChunkFile& data, ProgressTracking& progress);
            Storage* allocateOutput(int outputIndex = 0) const;
        };
        namespace Ops {
            class ImageInput : public GPUOperation {
            private:
                Storage* input;
                Storage* output;
                const Size size;
                std::vector<std::string> generateCode(GraphicPipeline& gpu, ChunkFile& data);
                void perform(GraphicPipeline& gpu, GL::ComputeProgram &program, const int workerIdx) const;
            public:
                ImageInput(Context& ctx, const std::string& name, const Size& size);
                int getInputCount()  const { return 1; }
                int getOutputCount() const { return 1; }
                Size getInputSize(int inputIndex = 0)   const { return size; }
                Size getOutputSize(int outputIndex = 0) const { return size; }
                void setInput(Storage& storage, int inputIndex = 0);
                void setOutput(Storage& storage, int outputIndex = 0);
                Storage::Type getOutputType(int outputIndex = 0) const;
                Storage* allocateOutput(int outputIndex = 0) const;
            };
        }

        /**
            A GPU operation having a single input and a single output
        */
        class FeedforwardGPUOperation : public GPUOperation {
        protected:
            const int outChannelsNum;
            const int outChanNumRoundUp4;
            Storage::Type inputType;
            Size inputSize;
            Storage* input;
            Storage* output;
            FeedforwardGPUOperation(
                Context& ctx,
                const std::string& name,
                const Size& inputSize,
                const int outChannelsNum
            );
            void perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int workerIdx) const;
        public:
            Storage::Type getInputType(int inputIndex = 0) const;
            Size getInputSize(int inputIndex = 0) const;
            void setInput(Storage& storage, int inputIndex = 0);
            void setOutput(Storage& storage, int outputIndex = 0);
        };
        class Feedforward2DGPUOperation : public FeedforwardGPUOperation {
        protected:
            Size kernelSize;
            Size stride;
            Size::Padding padding;
            Feedforward2DGPUOperation(
                Context& ctx,
                const std::string& name,
                const Size& inputSize,
                const int outChannelsNum,
                const Size& kernelSize,
                const Size& stride = Size::ONES,
                const Size::Padding& padding = Size::Padding::SAME
            );
        public:
            Size getOutputSize(int outputIndex = 0) const;
        };
        class Probe : public GPUOperation {
        private:
            Storage* input;
            AbstractBitmap* output;
            const Size size;
            Storage::Type inputType;
            int laneIndex;
            std::vector<std::string> generateCode(GraphicPipeline& gpu, ChunkFile& data);
            void perform(GraphicPipeline& gpu, GL::ComputeProgram &program, const int workerIdx) const;
            bool setInputType(Storage::Type type, int inputIndex = 0);
            void setOutput(Storage& storage, int outputIndex = 0);
            Storage* allocateOutput(int outputIndex = 0) const;
        public:
            Probe(Context& ctx, const std::string& name, const Size& size, int laneIndex);
            ~Probe();
            int getInputCount()  const { return 1; }
            int getOutputCount() const { return 1; }
            Size getInputSize(int inputIndex = 0)   const { return size; }
            Size getOutputSize(int outputIndex = 0) const { return size; }
            void setInput(Storage& input, int inputIndex = 0);
            AbstractBitmap& getOutput() { return *output; }
        };
    }
}

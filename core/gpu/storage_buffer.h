/**
    Abstraction form OpenGL's shared storage buffer object (SSBO)
 */
#pragma once
#include "../environment.h"
#include "../gpu/gpu_task.h"
namespace Beatmup {
    namespace GL {
        class StorageBuffer {
            friend class AbstractProgram;
        private:
            const size_t width, height, depth, entrySize;
            Environment& env;
            glhandle handle;
        public:
            StorageBuffer(Environment& env, size_t width, size_t height, size_t depth, const size_t entrySize);
            ~StorageBuffer();
            void allocate(GraphicPipeline& gpu, const void* data = nullptr);
            void bind(GraphicPipeline& gpu, int unit) const;
            void fetch(GraphicPipeline& gpu, void* data, size_t limit);
            inline size_t getWidth()  const { return width; }
            inline size_t getHeight() const { return height; }
            inline size_t getDepth()  const { return depth; }
            inline size_t getEntrySize() const { return entrySize; }
        };
        class StorageBufferFetcher : public GpuTask {
        private:
            StorageBuffer& buffer;
            void* outputBuffer;
            const size_t outputBufferSize;
            bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        public:
            StorageBufferFetcher(StorageBuffer& buffer, void* outputBuffer, size_t outputBufferSize);
        };
    }}
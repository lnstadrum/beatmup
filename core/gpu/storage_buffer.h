/**
    Abstraction form OpenGL's shared storage buffer object (SSBO)
 */
#pragma once
#include "../context.h"
#include "../gpu/gpu_task.h"
namespace Beatmup {
    namespace GL {
        class StorageBuffer {
            friend class AbstractProgram;
        private:
            size_t sizeBytes;
            GL::RecycleBin& recycleBin;
            glhandle handle;
        public:
            StorageBuffer(GL::RecycleBin& recycleBin);
            ~StorageBuffer();
            void allocate(GraphicPipeline& gpu, const size_t sizeBytes, const void* data = nullptr);
            void bind(GraphicPipeline& gpu, int unit) const;
            void fetch(GraphicPipeline& gpu, void* data, size_t limit);
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
    }
}
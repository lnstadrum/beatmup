/**
    Abstraction form OpenGL's shared storage buffer object (SSBO)
 */
#pragma once
#include "../context.h"
#include "gpu_task.h"
#include "../bitmap/abstract_bitmap.h"

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
            void allocateStatic(GraphicPipeline& gpu, const size_t sizeBytes, const void* data);
            void bind(GraphicPipeline& gpu, int unit) const;
            void fetch(GraphicPipeline& gpu, void* data, size_t limit);

            /**
                Copies buffer content to a bitmap
                \param[in] gpu        Graphic pipeline instance
                \param[in] offset     Offset in the buffer in bytes
                \param[in] stride     Stride between entries to be copied to the bitmap in the buffer in bytes
                \param[out] bitmap    The bitmap to fill
            */
            void fetchToBitmap(GraphicPipeline& gpu, size_t offset, size_t stride, AbstractBitmap& bitmap);

            inline size_t getCurrentCapacity() const { return sizeBytes; }
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
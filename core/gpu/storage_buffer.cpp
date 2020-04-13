#include "storage_buffer.h"
#include "bgl.h"
#include "recycle_bin.h"
#include "../exception.h"

#ifndef BEATMUP_OPENGLVERSION_GLES20

using namespace Beatmup;
using namespace GL;

StorageBuffer::StorageBuffer(GL::RecycleBin& recycleBin):
    sizeBytes(0), recycleBin(recycleBin), handle(0)
{}


StorageBuffer::~StorageBuffer() {
    class Deleter : public GL::RecycleBin::Item {
    private:
        const glhandle handle;
    public:
        Deleter(glhandle handle) : handle(handle) {}
        ~Deleter() {
            glDeleteBuffers(1, &handle);
        }
    };

    if (handle)
        recycleBin.put(new Deleter(handle));
}


void StorageBuffer::allocate(GraphicPipeline& gpu, const size_t sizeBytes, const void* data) {
    if (handle && this->sizeBytes != sizeBytes) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }

    if (!handle && sizeBytes > 0) {
        glGenBuffers(1, &handle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeBytes, data, GL_DYNAMIC_COPY);
    }

    this->sizeBytes = sizeBytes;
}


void StorageBuffer::bind(GraphicPipeline& gpu, int unit) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, handle);
    GLException::check("binding storage buffer");
}


void StorageBuffer::fetch(GraphicPipeline& gpu, void* data, size_t limit) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    void* buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, limit, GL_MAP_READ_BIT);
    const bool okay = buffer != nullptr;
    if (okay)
        memcpy(data, buffer, limit);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLException::check("reading storage buffer");
    if (!okay)
        throw RuntimeError("Buffer data not available");
}



StorageBufferFetcher::StorageBufferFetcher(StorageBuffer& buffer, void* outputBuffer, size_t outputBufferSize) :
    buffer(buffer), outputBuffer(outputBuffer), outputBufferSize(outputBufferSize)
{}


bool StorageBufferFetcher::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    buffer.fetch(gpu, outputBuffer, outputBufferSize);
    return true;
}

#endif
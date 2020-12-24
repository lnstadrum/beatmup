/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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

#include "storage_buffer.h"
#include "bgl.h"
#include "recycle_bin.h"
#include "../exception.h"
#include <cstring>

#ifndef BEATMUP_OPENGLVERSION_GLES20

using namespace Beatmup;
using namespace GL;

StorageBuffer::StorageBuffer(GL::RecycleBin& recycleBin):
    sizeBytes(0), recycleBin(recycleBin), handle(0)
{}


StorageBuffer::~StorageBuffer() {
    class Deleter : public GL::RecycleBin::Item {
    private:
        const handle_t handle;
    public:
        Deleter(handle_t handle) : handle(handle) {}
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

void Beatmup::GL::StorageBuffer::allocateStatic(GraphicPipeline& gpu, const size_t sizeBytes, const void* data) {
    if (handle && this->sizeBytes != sizeBytes) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }

    if (!handle && sizeBytes > 0) {
        glGenBuffers(1, &handle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeBytes, data, GL_STATIC_DRAW);
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

void Beatmup::GL::StorageBuffer::fetchToBitmap(GraphicPipeline& gpu, size_t offset, size_t stride, AbstractBitmap& bitmap) {
    Beatmup::RuntimeError::check(!bitmap.isMask(), "Mask bitmaps are not supported");

    const size_t limit = offset + stride * (bitmap.getSize().numPixels() - 1) + bitmap.getBitsPerPixel() / 8;
    Beatmup::RuntimeError::check(getCurrentCapacity() >= limit, "Bitmap does not fit the buffer content");
    AbstractBitmap::WriteLock<ProcessingTarget::CPU> lock(bitmap);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    const pixbyte* buffer = (const pixbyte*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, limit - offset, GL_MAP_READ_BIT);
    const bool okay = buffer != nullptr;
    if (okay) {
        pixbyte* ptr = bitmap.getData(0, 0);
        const int
            nPix = bitmap.getSize().numPixels(),
            step = bitmap.getBitsPerPixel() / 8;
        for (int i = 0; i < nPix; ++i, ptr += step, buffer += stride)
            memcpy(ptr, buffer, step);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



StorageBufferFetcher::StorageBufferFetcher(StorageBuffer& buffer, void* outputBuffer, size_t outputBufferSize) :
    buffer(buffer), outputBuffer(outputBuffer), outputBufferSize(outputBufferSize)
{}


bool StorageBufferFetcher::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    buffer.fetch(gpu, outputBuffer, outputBufferSize);
    return true;
}

#endif

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

#include "chunk_collection.h"
#include <stdexcept>
#include <algorithm>

using namespace Beatmup;

namespace Internal {

    /**
        Copies data from multidimensional strided tensor to a continuous memory space.
        Makes use of densely packed dimensions for a faster copy.
    */
    class Copy {
    private:
        const pybind11::buffer_info& info;
        int firstStridedDim;    //!< first non-dense dimension
        size_t packetSize;      //!< largest continuous packet size
        uint8_t* outPtr;        //!< output pointer
        size_t limit;           //!< number of bytes remaining to copy


        /**
            \internal
            Searches for first strided (non-dense) dimension.
        */
        size_t discover(int dim = 0) {
            size_t entrySize;
            if (dim + 1 < info.ndim) {
                entrySize = discover(dim + 1);
                if (entrySize == 0)
                    return 0;
            }
            else
                entrySize = info.itemsize;

            // check if densely packed
            if (info.strides[dim] == entrySize) {
                if (dim == 0) {
                    // all dimensions are densely packed
                    firstStridedDim = -1;
                    packetSize = info.size * info.itemsize;
                    return 0;
                }
                else
                    // just return dimension size
                    return entrySize * info.shape[dim];
            }

            // not densely packed, so copy by pieces
            firstStridedDim = dim;
            packetSize = entrySize;
            return 0;
        }


        /**
            \internal
            Recursively copies strided data to output.
        */
        void copy(const uint8_t* inPtr, int dim = 0) {
            if (dim > firstStridedDim) {
                // densely packed dimensions follow
                size_t size = std::min(limit, packetSize);
                memcpy(outPtr, inPtr, size);
                outPtr += size;
                limit -= size;
            }
            else
                for (size_t i = 0; i < info.shape[dim]; ++i)
                    copy(inPtr + i * info.strides[dim], dim + 1);
        }

    public:
        Copy(const pybind11::buffer_info& info, void* outputBuffer, size_t limit):
            info(info), outPtr(static_cast<uint8_t*>(outputBuffer)), limit(limit)
        {
            discover();
            copy(static_cast<const uint8_t*>(info.ptr));
        }

        inline size_t remaining() const { return limit; }
    };

}

chunksize_t Python::WritableChunkCollection::chunkSize(const std::string& id) const {
    auto it = data.find(id);
    if (it == data.cend())
        return 0;
    auto info = it->second.request();
    return info.size * info.itemsize;
}


chunksize_t Python::WritableChunkCollection::fetch(const std::string& id, void* data, const chunksize_t limit) {
    auto it = this->data.find(id);
    if (it == this->data.end())
        return 0;
    Internal::Copy copy(it->second.request(), data, limit);
    return limit - copy.remaining();
}


void Python::WritableChunkCollection::save(const std::string& filename, bool append) {
    ChunkFileWriter writer(filename, append);
    for (auto it : data) {
        Chunk chunk(*this, it.first);
        writer(it.first, chunk(), chunk.size());
    }
}
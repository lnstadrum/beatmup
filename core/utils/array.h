/**
    Stuff
 */

#pragma once
#include <vector>

namespace Beatmup {
    template<typename datatype, size_t numdim> class Array {
    private:
        std::vector<datatype> storage;
        size_t dims[numdim];
    public:
        Array(size_t length) :
            storage(length)
        {
            static_assert(numdim == 1, "Dimensions mismatch");
            dims[0] = length;
        }

        Array(size_t width, size_t height) :
            storage(width * height)
        {
            static_assert(numdim == 2, "Dimensions mismatch");
            dims[0] = width;
            dims[1] = height;
        }

        Array(size_t width, size_t height, size_t depth):
            storage(width * height * depth)
        {
            static_assert(numdim == 3, "Dimensions mismatch");
            dims[0] = width;
            dims[1] = height;
            dims[2] = depth;
        }

        Array(size_t channels, size_t width, size_t height, size_t depth) :
            storage(channels * width * height * depth)
        {
            static_assert(numdim == 4, "Dimensions mismatch");
            dims[0] = channels;
            dims[1] = width;
            dims[2] = height;
            dims[3] = depth;
        }

        datatype* data() { return storage.data(); }
        size_t size() const { return storage.size(); }

        datatype& operator()(size_t i) {
            static_assert(numdim == 1, "Dimensions mismatch");
            return storage[i];
        }

        datatype& operator()(size_t x, size_t y) {
            static_assert(numdim == 2, "Dimensions mismatch");
            return storage[y * dims[0] + x];
        }

        datatype& operator()(size_t x, size_t y, size_t z) {
            static_assert(numdim == 3, "Dimensions mismatch");
            return storage[(z * dims[1] + y) * dims[0] + x];
        }

        datatype& operator()(size_t c, size_t x, size_t y, size_t z) {
            static_assert(numdim == 4, "Dimensions mismatch");
            return storage[((z * dims[2] + y) * dims[1] + x) * dims[0] + c];
        }

        const datatype& operator()(size_t c, size_t x, size_t y, size_t z) const {
            static_assert(numdim == 4, "Dimensions mismatch");
            return storage[((z * dims[2] + y) * dims[1] + x) * dims[0] + c];
        }
    };
}
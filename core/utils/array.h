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

#pragma once
#include <vector>

namespace Beatmup {
    /**
        Multidimensional array container with data access facilities
        The number of dimensions is compile time-known.
    */
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
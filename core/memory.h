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
#include <cstddef>
#include <cstdint>

namespace Beatmup {
    /**
        Aligned memory buffer
    */
    class AlignedMemory {
    private:
        void* rawAddr;
        void* alignAddr;
        AlignedMemory(const AlignedMemory&) = delete;
    public:
        static const size_t DEFAULT_ALIGNMENT;      //!< default number of bytes to align the address

        AlignedMemory() : rawAddr(nullptr), alignAddr(nullptr) {}
        AlignedMemory(size_t size, size_t align = DEFAULT_ALIGNMENT);
        AlignedMemory(size_t size, int value, size_t align = DEFAULT_ALIGNMENT);
        ~AlignedMemory();

        AlignedMemory& operator=(AlignedMemory&&);

        inline void* operator()() { return alignAddr; }

        inline const void* operator()() const { return alignAddr; }

        template<typename datatype> inline datatype* ptr(int offset = 0) {
            return static_cast<datatype*>(alignAddr) + offset;
        }

        template<typename datatype> inline const datatype* ptr(int offset = 0) const {
            return static_cast<const datatype*>(alignAddr) + offset;
        }

        template<typename datatype> inline datatype& at(int offset) {
            return *(static_cast<datatype*>(alignAddr) + offset);
        }

        template<typename datatype> inline const datatype& at(int offset) const {
            return *(static_cast<datatype*>(alignAddr) + offset);
        }

        inline unsigned char operator[](int offset) const { return at<unsigned char>(offset); }

        inline operator bool() const { return rawAddr != nullptr; }

        /**
            Frees the allocated memory.
        */
        void free();

        /**
            Returns the size of available (free) operating memory in bytes
        */
        static uint64_t available();

        /**
            Returns the size of total operating memory in bytes
        */
        static uint64_t total();
    };
}

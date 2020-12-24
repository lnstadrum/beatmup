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
#include "../basic_types.h"
#include <fstream>

namespace Beatmup {
    /**
        Minimal input stream interface
    */
    class InputStream {
    public:
        /**
            Reads a given number of bytes into a specific memory location.
            \param[in] buffer       The address to store the data to
            \param[in] bytes        Number of bytes to read
            \return `true` on success.
        */
        virtual bool operator()(void * buffer, msize bytes) = 0;

        /**
            Moves the read pointer to a given position in the stream.
            \param pos      The position in bytes from the beginning of the stream
            \return `true` on success.
        */
        virtual bool seek(msize pos) = 0;

        /**
            Returns `true`, if the end of the stream is reached (i.e., all the data is read or the stream is empty).
        */
        virtual bool eof() const = 0;
    };


    /**
        InputStream reading from a file
    */
    class FileInputStream : public InputStream {
    private:
        std::ifstream stream;
    public:
        FileInputStream();
        inline FileInputStream(const char* filename) { open(filename); }

        bool isOpen() const;
        void open(const char* filename);
        void close();
        void clear();

        bool operator()(void * buffer, msize bytes);
        bool seek(msize pos);
        bool eof() const;
    };


    /**
        InputStream reading from memory
    */
    class MemoryInputStream : public InputStream {
    private:
        const char* data;
        const msize size;
        msize ptr;
    public:
        inline MemoryInputStream(const void* data, msize size): data(static_cast<const char*>(data)), size(size), ptr(0) {}

        bool operator()(void * buffer, msize bytes);
        bool seek(msize pos);
        bool eof() const;
    };
}
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

#include "input_stream.h"
#include <cstring>

using namespace Beatmup;

FileInputStream::FileInputStream() {}


bool FileInputStream::isOpen() const {
    return stream.is_open();
}

void FileInputStream::open(const char *filename) {
    stream.open(filename, std::ios::in | std::ios::binary);
}

void FileInputStream::close() {
    stream.close();
}

void FileInputStream::clear() {
    stream.clear();
}


bool FileInputStream::operator()(void *buffer, msize bytes) {
    stream.read((char*)buffer, bytes);
    return stream.good();
}

bool FileInputStream::seek(msize pos) {
    stream.seekg(pos);
    return stream.good();
}

bool FileInputStream::eof() const {
    return stream.eof();
}


bool MemoryInputStream::operator()(void * buffer, msize bytes) {
    const msize step = ptr + bytes < size ? bytes : size - ptr;
    memcpy(buffer, data + ptr, step);
    ptr += step;
    return step == bytes;
}

bool MemoryInputStream::seek(msize pos) {
    if (pos < size)
        ptr = pos;
    return pos < size;
}

bool MemoryInputStream::eof() const {
    return ptr >= size;
}

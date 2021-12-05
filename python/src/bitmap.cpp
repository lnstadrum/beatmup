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

#include "bitmap.h"
#include <stdexcept>

using namespace Beatmup;


Python::Bitmap::Bitmap(Beatmup::Context& context, pybind11::buffer& buffer):
    AbstractBitmap(context)
{
    auto info = buffer.request();

    // check if empty
    if (info.size == 0)
        throw std::invalid_argument("A bitmap cannot be empty");

    // check dimensions
    if (info.ndim < 3)
        throw std::invalid_argument("A bitmap is expected to have at least 3 dimensions");
    else
        for (int i = 0; i + 3 < info.ndim; ++i)
            if (info.shape[i] != 1)
                throw std::invalid_argument("A bitmap is expected to have at most 3 inner non-singleton dimensions.");
    
    // check pixel format
    static const char* FORMAT_UINT8 = "B";
    static const char* FORMAT_FLOAT = "f";
    const auto numChannels = info.shape[info.ndim - 1];
    
    if (info.format == FORMAT_UINT8)
        switch (numChannels) {
            case 1: format = PixelFormat::SingleByte; break;
            case 3: format = PixelFormat::TripleByte; break;
            case 4: format = PixelFormat::QuadByte; break;
            default:
                throw std::invalid_argument("A bitmap may have 1, 3 of 4 channels, but got " + std::to_string(numChannels));
        }
    else if (info.format == FORMAT_FLOAT)
        switch (numChannels) {
            case 1: format = PixelFormat::SingleFloat; break;
            case 3: format = PixelFormat::TripleFloat; break;
            case 4: format = PixelFormat::QuadFloat; break;
            default:
                throw std::invalid_argument("A bitmap may have 1, 3 of 4 channels, but got " + std::to_string(numChannels));
        }
    else
        throw std::invalid_argument("Unsupported data format. Expected bitmap values of type float32 or uint8.");

    // check stride
    const int bps = getBitsPerPixel() / 8;
    strideBytes = info.shape[info.ndim - 2] * bps;
    if (info.strides[info.ndim - 1] * numChannels != bps || info.strides[info.ndim - 2] != bps || info.strides[info.ndim - 3] != strideBytes)
        throw std::invalid_argument("Strided bitmaps are not supported.");

    // GOOOD.
    data = std::move(info);
}


const pixbyte* Python::Bitmap::getData(int x, int y) const { 
    return static_cast<const pixbyte*>(data.ptr) + y * strideBytes + x;
}


pixbyte* Python::Bitmap::getData(int x, int y) { 
    return static_cast<pixbyte*>(data.ptr) + y * strideBytes + x;
}


const msize Python::Bitmap::getMemorySize() const { 
    return data.itemsize * data.size;
}
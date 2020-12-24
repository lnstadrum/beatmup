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
#include <fstream>
#include <string>
#include <cstdint>

namespace Beatmup {
    /**
        Toolset to load and store images in BMP format.
    */
    class BmpFile {
    private:
#ifdef _MSC_VER
#pragma pack(push,1)
#endif
        typedef struct
#ifndef _MSC_VER
        __attribute__((packed))
#endif
        {
            uint8_t magic[2];
            uint32_t size;
            uint16_t reserved[2];
            uint32_t offset;
            uint32_t headerSize;
            int32_t  width, height;
            uint16_t numColorPlanes;
            uint16_t bpp;
            uint32_t compression;
            uint32_t imageSize;
            int32_t  hdpi, vdpi;
            uint32_t numImportantColors;
        } Header;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

        static const Header BMP_HEADER_REFERENCE;

        Header header;
        std::ifstream in;

    public:
        BmpFile(const char* filename);

        /**
            Loads the content of the file into memory.
            \param[out] pixels              The memory buffer to fill with pixel data.
            \param[in] pixelsSizeInBytes    The memory buffer size in bytes
        */
        void load(void* pixels, const uint32_t pixelsSizeInBytes);

        /**
            Stores an image into a BMP file.
            \param[in] pixels       Memory buffer containing the pixel data to save
            \param[in] width        Image width in pixels
            \param[in] height       Image height in pixels
            \param[in] bpp          Number of bits per pixel
            \param[in] filename     Name/path of the file to write the data to.
         */
        static void save(
            const void* pixels,
            int32_t width,
            int32_t height,
            uint8_t bpp,
            const char* filename
        );

        inline uint8_t getBitsPerPixel() const { return header.bpp; }
        inline int32_t getWidth()        const { return header.width; }
        inline int32_t getHeight()       const { return header.height; }
    };
}

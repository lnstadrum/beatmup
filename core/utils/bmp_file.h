/**
    BMP
*/
#pragma once
#include <fstream>
#include <string>
#include <stdint.h>

namespace Beatmup {
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
        void load(void* pixels, const uint32_t pixelsSizeInBytes);

        uint8_t getBitsPerPixel() const { return header.bpp; }
        int32_t getWidth()        const { return header.width; }
        int32_t getHeight()       const { return header.height; }

        static void save(
            const void* pixels,
            int32_t width,
            int32_t height,
            uint8_t bpp,
            const char* filename
        );
    };
}

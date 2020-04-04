/*
    Internally managed bitmap
*/

#pragma once

#include "../basic_types.h"
#include "../context.h"
#include "abstract_bitmap.h"
#include <vector>

namespace Beatmup {

    class InternalBitmap : public AbstractBitmap {
    private:
        PixelFormat pixelFormat;
        int width, height;
        memchunk memory;				//!< ID of memory chunk
        pixbyte* data;					//!< pointer to pixel data when locked

        void lockPixelData();
        void unlockPixelData();

    public:
        /**
            Creates an empty bitmap
            \param ctx            A Beatmup context
            \param pixelFormat    Pixel format
            \param width          Bitmap width in pixels
            \param height         Bitmap height in pixels
            \param allocate       If `true`, a storage is allocated in RAM. Othrewise the allocation is deferred
                                  till the first use of the bitmap data by CPU. It is convenient to not allocate
                                  if the bitmap is only used as a texture handler to store intremediate data when
                                  processing on GPU.
        */
        InternalBitmap(Context& ctx, PixelFormat pixelFormat, int width, int height, bool allocate = true);

        InternalBitmap(Context& ctx, const char* filename);

        virtual ~InternalBitmap();

        const PixelFormat getPixelFormat() const;
        const int getWidth() const;
        const int getHeight() const;
        const msize getMemorySize() const;
        pixbyte* getData(int x, int y) const;

        void saveBmp(const char* filename);
    };

}

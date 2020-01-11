/*
    Bitmap class
*/

#pragma once

#include "../basic_types.h"
#include "../geometry.h"
#include "../utils/image_resolution.h"
#include "../gpu/texture_handler.h"
#include "../exception.h"
#include <string>

namespace Beatmup {

    class Context;

    enum PixelFormat {
        SingleByte = 0,		//!< single channel of 8 bits per pixel (like grayscale), unsigned integer values
        TripleByte,			//!< 3 channels of 8 bits per pixel (like RGB), unsigned integer values
        QuadByte,			//!< 4 channels of 8 bits per pixel (like RGBA), unsigned integer values
        SingleFloat,		//!< single channel of 32 bits per pixel (like grayscale), single precision floating point values
        TripleFloat,		//!< 3 channels of 32 bits per pixel, single precision floating point values
        QuadFloat,			//!< 4 channels of 32 bits per pixel, single precision floating point values,
        BinaryMask,			//!< 1 bit per pixel
        QuaternaryMask,		//!< 2 bits per pixel
        HexMask				//!< 4 bits per pixel
    };


    class AbstractBitmap : public GL::TextureHandler {
        friend class GraphicPipeline;

    private:
        AbstractBitmap(const AbstractBitmap& that) = delete;		//!< disabling copying constructor

        bool pixelDataLocked;								//!< flag signaling whether the pixel buffer in CPU memory is currently locked
        bool upToDate[2];									//!< bitmap up-to-date state on CPU and GPU

    protected:
        AbstractBitmap(Context& ctx);

        Context& ctx;									//!< context managing this bitmap

        // overridden methods from TextureHandler
        virtual void prepare(GraphicPipeline& gpu);
        virtual const TextureFormat getTextureFormat() const;

        /**
            Locks access to the memory buffer in CPU memory, containing pixel data
        */
        virtual void lockPixelData() = 0;

        /**
            Unlocks access to the memory buffer in CPU memory, containing pixel data
        */
        virtual void unlockPixelData() = 0;

    public:
        static const char* PIXEL_FORMAT_NAMES[9];			//!< pixel format names
        static const unsigned char CHANNELS_PER_PIXEL[9];	//!< number of channels for each pixel format
        static const unsigned char BITS_PER_PIXEL[9];		//!< number of bits per pixel for each pixel format

        const int getDepth() const { return 1; }

        /**
            Pixel format of the bitmap
        */
        virtual const PixelFormat getPixelFormat() const = 0;

        /**
            Bitmap size in bytes
        */
        virtual const msize getMemorySize() const = 0;

        /**
            Unlocks access to the memory buffer (in CPU memory) containing pixel data
        */
        virtual void unlockPixels();

        /**
            Locks access to the pixel data guaranteeing that the bitmap is up to date on the required processing target
        */
        virtual void lockPixels(ProcessingTarget);

        /**
            Marks as out-of-date bitmap content stored specified target processing unit memory
        */
        virtual void invalidate(ProcessingTarget);

        bool isUpToDate(ProcessingTarget) const;

        bool isDirty() const;

        /**
            Returns a pointer to given pixel.
            \param x			target pixel horizontal coordinate
            \param y			target pixel vertical coordinate
            \returns a pointer, may be NULL.
        */
        virtual pixbyte* getData(int x, int y) const = 0;

        /**
            Retrieves integer value of given channel at given pixel
            \param x			target pixel horizontal coordinate
            \param y			target pixel vertical coordinate
            \param cha			target channel
            \returns requested pixel value
        */
        int getPixelInt(int x, int y, int cha = 0) const;

        /**
            Returns number of bits per pixel stored in each bitmap.
        */
        const unsigned char getBitsPerPixel() const;

        /**
            Returns number of bytes per pixel stored in each bitmap.
        */
        const unsigned char getNumberOfChannels() const;

        /**
            Returns the bitmap resolution within ImageResolution object
        */
        const ImageResolution getSize() const;

        Context& getContext() const;

        /**
            Sets all the pixels to zero
        */
        void zero();

        /**
            Returns `true` if the bitmap contains integer values, `false` otherwise
        */
        bool isInteger() const;

        /**
            Returns `true` if the bitmap contains floating point values, `false` otherwise
        */
        bool isFloat() const;

        /**
            Returns `true` if the bitmap is a mask, `false` otherwise
        */
        bool isMask() const;

        /**
            Returns `true` if a given pixel format corresponds to integer values, `false` otherwise
        */
        static bool isInteger(PixelFormat pixelFormat);

        /**
            Returns `true` if a given pixel format corresponds to floating point values, `false` otherwise
        */
        static bool isFloat(PixelFormat pixelFormat);

        /**
            Returns `true` if a given pixel format corresponds to a mask, `false` otherwise
        */
        static bool isMask(PixelFormat pixelFormat);

        /**
            Retruns string description of a bitmap
        */
        std::string toString() const;

        ~AbstractBitmap();
    };


    typedef AbstractBitmap* BitmapPtr;
}

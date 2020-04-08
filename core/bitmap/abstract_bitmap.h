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

    enum PixelFlow {
        CpuRead  = 1 << 0,
        GpuRead  = 1 << 1,
        CpuWrite = 1 << 2,
        GpuWrite = 1 << 3
    };

    class AbstractBitmap : public GL::TextureHandler {
        friend class GraphicPipeline;

    private:
        AbstractBitmap(const AbstractBitmap& that) = delete;		//!< disabling copying constructor

    protected:
        Context& ctx;									//!< context managing this bitmap
        bool upToDate[2];									//!< bitmap up-to-date state on CPU and GPU

        AbstractBitmap(Context& ctx);

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
        static const int NUM_PIXEL_FORMATS = 9;
        static const char* PIXEL_FORMAT_NAMES[NUM_PIXEL_FORMATS];            //!< pixel format names
        static const unsigned char CHANNELS_PER_PIXEL[NUM_PIXEL_FORMATS];    //!< number of channels for each pixel format
        static const unsigned char BITS_PER_PIXEL[NUM_PIXEL_FORMATS];        //!< number of bits per pixel for each pixel format

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
            Prepares the bitmap for a specific processing; enables direct pixel acces for processing units that will
            operate on the bitmap.
        */
        void lockContent(PixelFlow flow);

        /**
            Shortcut for locking the content in function of GPU availablility and a reading/writing flag.
        */
        inline void lockContent(GraphicPipeline* gpu, bool write) {
            lockContent(gpu ?
                (write ? PixelFlow::GpuWrite : PixelFlow::GpuRead) :
                (write ? PixelFlow::CpuWrite : PixelFlow::CpuRead));
        }

        /**
            Releases access to pixels after a specific process.
            This operation may release memory. The pixel flow value must correspond to value given in lockContent(..),
            otherwise updated pixel data may be lost.
        */
        void unlockContent(PixelFlow flow);

        /**
            Shortcut for unlocking the content in function of GPU availablility and a reading/writing flag.
        */
        void unlockContent(GraphicPipeline* gpu, bool write) {
            unlockContent(gpu ?
                (write ? PixelFlow::GpuWrite : PixelFlow::GpuRead) :
                (write ? PixelFlow::CpuWrite : PixelFlow::CpuRead));
        }


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
        const ImageResolution getSize() const {
            return ImageResolution(getWidth(), getHeight());
        }

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


        /**
            Guard utility for locking and unlocking content
        */
        class ContentLock {
        private:
            AbstractBitmap& bitmap;
            const PixelFlow flow;
        public:
            ContentLock(AbstractBitmap& bitmap, const PixelFlow flow):
                bitmap(bitmap), flow(flow)
            {
                bitmap.lockContent(flow);
            }

            ~ContentLock() {
                bitmap.unlockContent(flow);
            }
        };


        /**
            Locks a bitmap for reading checking whether it is up-to-date.
            Once instantiated on a bitmap, bitmap.getData(..) returns a valid address. Modifying its content puts
            the bitmap into inconsistent state.
        */
        class ReadLock {
        private:
            AbstractBitmap& bitmap;
            const PixelFlow flow;
        public:
            ReadLock(AbstractBitmap& bitmap, ProcessingTarget unit = ProcessingTarget::CPU):
                bitmap(bitmap), flow(unit == ProcessingTarget::CPU ? PixelFlow::CpuRead : PixelFlow::GpuRead)
            {
                bitmap.lockContent(flow);
            }

            ~ReadLock() {
                bitmap.unlockContent(flow);
            }
        };


        /**
            Makes a bitmap writable CPU access regardless its actual state.
            Once instantiated on a bitmap, bitmap.getData(..) returns valid address. When destroyed, marks the bitmap
            being up-to-date in RAM and outdated in GPU memory.
        */
        class WriteLock {
        private:
            AbstractBitmap& bitmap;
        public:
            WriteLock(AbstractBitmap& bitmap):
                bitmap(bitmap)
            {
                bitmap.lockContent(PixelFlow::CpuWrite);
            }

            ~WriteLock() {
                bitmap.unlockContent(PixelFlow::CpuWrite);
            }
        };
    };


    inline PixelFlow operator + (PixelFlow a, PixelFlow b) {
        return static_cast<PixelFlow>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline bool operator & (PixelFlow a, PixelFlow b) {
        return (static_cast<int>(a) & static_cast<int>(b)) > 0;
    }
}

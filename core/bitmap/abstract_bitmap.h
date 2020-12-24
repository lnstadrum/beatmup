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

#include "content_lock.h"
#include "../basic_types.h"
#include "../geometry.h"
#include "../utils/image_resolution.h"
#include "../gpu/texture_handler.h"
#include "../exception.h"
#include <string>

namespace Beatmup {

    /** \page ProgrammingModel
        \section secBitmaps Bitmaps
        Since %Beatmup is mainly oriented towards image processing, AbstractBitmap is another central class in %Beatmup.

        An AbstractBitmap is basically an image. From the application perspective it has two main implementations.
         - InternalBitmap is a platform-independent image managed by %Beatmup itself. It is convenient to be use to exchange data between tasks.
         - Platform/frontend-dependent implementations, such as Android::Bitmap, Android::ExternalBitmap, or Python::Bitmap. This is used for I/O
           operations with the outer world and typically implements a direct access to the pixel data without memory copy.

        %Beatmup is thought to be lightweight and dependency-free. For this reason it does not incorporate image decoding/encoding features: it
        cannot natively read and write JPEG or PNG files for example. This is not a problem when using %Beatmup within an application where all the
        typical means of loading and storing images are accessible through the corresponding AbstractBitmap implementations. Also, for debugging
        purposes and minimal I/O capabilities %Beatmup supports reading and writing BMP files.

        \subsection ssecDevice CPU and GPU
        %Beatmup uses GPU to process images when possible. In order to mix efficiently CPU and GPU processing, %Beatmup can store the same image
        in CPU memory, GPU memory or both. This naturally implies pixel transfer operations. Internally, %Beatmup hides this from the user as much
        as possible and only performs the pixel data transfer when needed. However, when it comes to exchange the image data with the application
        code, the user typically needs to make sure the CPU version of the image (the one accessible with the platform-specific bitmaps outside of
        the %Beatmup environment) is up-to-date with respect to the GPU version used by %Beatmup.

         - AbstractBitmap::isUpToDate() function allows to check if the bitmap is up-to-date for a specific device (GPU or CPU).
         - If it needs to be updated, Swapper is an AbstractTask copying the image data between CPU and GPU. Namely, Swapper::pullPixels()
           updates the CPU version of an image after it is processed, so that it can be consumed by the applications using the platform-dependent
           bitmaps.
         - Some tasks offer special tools to ensure that CPU has access to the bitmap content up-to-date, e.g
           SceneRenderer::setOutputPixelsFetching().

        \subsection ssecGpuGarbage GPU garbage collection
        When a bitmap is destroyed in the application code, its GPU storage is not destroyed immediately. This is due to the fact that destroying a
        texture representing the bitmap content in the GPU memory needs to be done in a thread that has access to the GPU, which is one of the
        threads in the thread pool. The textures of destroyed bitmaps are marked as unused anymore and put into a "GPU trash bin". The latter is
        emptied by calling GL::RecycleBin::emptyBin() function on a recycle bin object instance returned by Context::getGpuRecycleBin(). Note that
        the recycle bin instance is only allocated is the GPU is actually used within the given Context.

        In applications doing repeated allocations and deallocations of images (e.g., processing video frames in a loop), it is recommended to empty
        the GPU recycle bin periodically in the described way in order to prevent running out of memory.
    */

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


    /**
        A very basic class for any image.
        Contains interfaces to access the bitmap information and its content.
    */
    class AbstractBitmap : public GL::TextureHandler {
    public:
        class ReadLock;
        template<const ProcessingTarget> class WriteLock;

    private:
        friend class GraphicPipeline;
        friend class BitmapContentLock;
        friend class AbstractBitmap::ReadLock;
        friend class AbstractBitmap::WriteLock<ProcessingTarget::CPU>;
        friend class AbstractBitmap::WriteLock<ProcessingTarget::GPU>;

        AbstractBitmap(const AbstractBitmap& that) = delete;		//!< disabling copying constructor

    protected:
        Context& ctx;									//!< context managing this bitmap
        bool upToDate[2];									//!< bitmap up-to-date state on CPU and GPU

        AbstractBitmap(Context& ctx);

        /**
            Locks access to the CPU memory buffer containing pixel data
        */
        virtual void lockPixelData() = 0;

        /**
            Unlocks access to the CPU memory buffer containing pixel data
        */
        virtual void unlockPixelData() = 0;

        // overridden methods from TextureHandler
        virtual void prepare(GraphicPipeline& gpu);

    public:
        static const int NUM_PIXEL_FORMATS = 9;
        static const char* PIXEL_FORMAT_NAMES[NUM_PIXEL_FORMATS];            //!< pixel format names
        static const unsigned char CHANNELS_PER_PIXEL[NUM_PIXEL_FORMATS];    //!< number of channels for each pixel format
        static const unsigned char BITS_PER_PIXEL[NUM_PIXEL_FORMATS];        //!< number of bits per pixel for each pixel format

        // overridden methods from TextureHandler
        virtual const int getDepth() const { return 1; }
        virtual const TextureFormat getTextureFormat() const;

        /**
            Pixel format of the bitmap
        */
        virtual const PixelFormat getPixelFormat() const = 0;

        /**
            Bitmap size in bytes
        */
        virtual const msize getMemorySize() const = 0;

        bool isUpToDate(ProcessingTarget) const;

        /**
            Returns `true` if the bitmap does not contain any valid content.
        */
        bool isDirty() const;

        /**
            Returns a pointer to given pixel.
            \param x			target pixel horizontal coordinate
            \param y			target pixel vertical coordinate
            \returns a pointer, may be NULL.
        */
        virtual const pixbyte* getData(int x, int y) const = 0;
        virtual pixbyte* getData(int x, int y) = 0;

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
            Retruns a string describing the bitmap
        */
        std::string toString() const;

        /**
            Saves the bitmap to a BMP file
         */
        void saveBmp(const char* filename);

        ~AbstractBitmap();

        /**
            Locks a bitmap for reading on CPU.
            Once instantiated on a bitmap, bitmap.getData() returns a valid address. Modifying its content puts the bitmap into inconsistent state.
            Warning: using this lock in a task (not in the user code) may cause a dead lock.
        */
        class ReadLock {
        private:
            AbstractBitmap& bitmap;
        public:
            ReadLock(AbstractBitmap& bitmap);
            ~ReadLock();
        };


        /**
            Makes a bitmap writable for a specific target device.
            Once instantiated on a bitmap, bitmap.getData() returns valid address. When destroyed, marks the bitmap being up-to-date in RAM and
            outdated in GPU memory.
            This lock may be used within tasks.
        */
        template<ProcessingTarget target>
        class WriteLock {
        private:
            AbstractBitmap& bitmap;
        public:
            WriteLock(AbstractBitmap& bitmap) : bitmap(bitmap) {
                if (target == ProcessingTarget::CPU)
                    bitmap.lockPixelData();
            }

            ~WriteLock() {
                bitmap.upToDate[ProcessingTarget::GPU] = (target == ProcessingTarget::GPU);
                bitmap.upToDate[ProcessingTarget::CPU] = (target == ProcessingTarget::CPU);
                if (target == ProcessingTarget::CPU)
                    bitmap.unlockPixelData();
            }
        };

    };
}

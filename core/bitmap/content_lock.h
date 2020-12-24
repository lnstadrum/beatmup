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
#include "basic_types.h"
#include <initializer_list>
#include <map>

namespace Beatmup {
    class GraphicPipeline;
    class AbstractBitmap;

    /**
     * Makes sure the bitmap content is accessible within an image processing task.
     * To enable direct access to pixels, a specific operation of locking the bitmap content is typically required. When done, the bitmap content needs to be unlocked.
     * Depending on the access type (reading/writing) an the processing target (CPU or GPU), a memory allocation and/or data transfer may be required.
     * BitmapContentLock performs the data transfer if necessary. It also handles multiple locking of the same bitmap by reference counting.
     */
    class BitmapContentLock {
        BitmapContentLock(const BitmapContentLock&) = delete;
    
    private:
        typedef struct {
            bool read, write;
            bool cpu, gpu;
            bool isDataLocked;  //!< if `true`, the bitmap pixel data is locked in RAM
            int refs;           //!< number of times the bitmap is locked
        } LockDescriptor;

        std::map<AbstractBitmap*, LockDescriptor> bitmaps;

    public:
        BitmapContentLock() {}
        ~BitmapContentLock();

        /**
            Locks content of a bitmap for reading using a specific processing target device.
            If the lock is already acquired, only increases the counter.
            If the bitmap was previously locked for a different target device, an exception is thrown.
            \param gpu      A graphic pipeline instance. Used to transfer the pixel data between CPU and GPU if needed.
            \param bitmap   The bitmap to lock
            \param target   Target processing device to make the pixel content readable for (CPU or GPU)
        */
        void readLock(GraphicPipeline* gpu, AbstractBitmap* bitmap, ProcessingTarget target);

        /**
            Locks content of a bitmap for writing using a specific processing target device.
            If the lock is already acquired, only increases the counter.
            If the bitmap was previously locked for reading, an exception is thrown.
            However, it is allowed to lock for different devices, or to lock for writing first and reading later.
            \param gpu      A graphic pipeline instance
            \param bitmap   The bitmap to lock
            \param target   Target processing device to make the pixel content readable for (CPU or GPU)
        */
        void writeLock(GraphicPipeline* gpu, AbstractBitmap* bitmap, ProcessingTarget target);

        /**
            Drops a lock to the bitmap.
            If no other locks own the content, the bitmap is unlocked.
        */
        void unlock(AbstractBitmap* bitmap);

        /**
            Unlocks all the locked bitmaps unconditionally.
        */
        void unlockAll();

        template <const ProcessingTarget target>
        inline void lock(GraphicPipeline* gpu, AbstractBitmap* input, AbstractBitmap* output) {
            writeLock(gpu, output, target);
            readLock(gpu, input, target);
        }

        inline void lock(GraphicPipeline* gpu, ProcessingTarget target, AbstractBitmap* input, AbstractBitmap* output) {
            writeLock(gpu, output, target);
            readLock(gpu, input, target);
        }

        template <const ProcessingTarget target>
        inline void lock(GraphicPipeline* gpu, std::initializer_list<AbstractBitmap*> read, std::initializer_list<AbstractBitmap*> write) {
            for (auto bmp : write)
                writeLock(gpu, bmp, target);
            for (auto bmp : read)
                readLock(gpu, bmp, target);
        }

        template<typename ... Args>
        inline void unlock(AbstractBitmap* first, Args ... others) {
            unlock(first);
            unlock(others...);
        }
    };
}
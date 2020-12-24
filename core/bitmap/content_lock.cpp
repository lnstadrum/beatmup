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

#include "content_lock.h"
#include "abstract_bitmap.h"
#include "../gpu/pipeline.h"
#ifdef BEATMUP_DEBUG
#include "../debug.h"
#endif

using namespace Beatmup;


BitmapContentLock::~BitmapContentLock() {
#ifdef BEATMUP_DEBUG
    if (!bitmaps.empty())
        BEATMUP_DEBUG_E("Destroying a content lock having %d locked bitmaps", (int)bitmaps.size());
#endif
}


void BitmapContentLock::readLock(GraphicPipeline* gpu, AbstractBitmap* bitmap, ProcessingTarget target) {
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(!bitmap->isDirty(), "Reading a dirty bitmap");
    if (target == ProcessingTarget::GPU)
        DebugAssertion::check(gpu, "Cannot lock a bitmap on GPU: no graphic pipeline instance available");
#endif

    bool transferToGpuNeeded = false;
    bool transferFromGpuNeeded = false;

    auto it = bitmaps.find(bitmap);
    if (it == bitmaps.end()) {
        // creating a lock descriptor
        LockDescriptor lock{
            true, false,
            target == ProcessingTarget::CPU,
            target == ProcessingTarget::GPU,
            false,
            1
        };

        // pixel transfer to GPU is needed if the bitmap is not up to date on GPU
        transferToGpuNeeded = lock.gpu && !bitmap->isUpToDate(ProcessingTarget::GPU);
        transferFromGpuNeeded = lock.cpu && !bitmap->isUpToDate(ProcessingTarget::CPU);

        // locking pixel data in RAM
        if (lock.cpu || transferToGpuNeeded || transferFromGpuNeeded) {
            bitmap->lockPixelData();
            lock.isDataLocked = true;
        }
        
        bitmaps.emplace(bitmap, lock);
    }

    else {
        // already locked, just increase reference counter
        auto& lock = it->second;

        RuntimeError::check(((target == ProcessingTarget::CPU) == lock.cpu) && ((target == ProcessingTarget::GPU) == lock.gpu),
            "Lock target mismatch: a bitmap was previously locked for a different target");

        // It is allowed to lock a bitmap for writing first and for reading later.
        if (!lock.read) {
            transferToGpuNeeded = lock.gpu && !bitmap->isUpToDate(ProcessingTarget::GPU);
            transferFromGpuNeeded = lock.cpu && !bitmap->isUpToDate(ProcessingTarget::CPU);

            if (!lock.isDataLocked && (transferToGpuNeeded || transferFromGpuNeeded)) {
                bitmap->lockPixelData();
                lock.isDataLocked = true;
            }

            lock.read = true;
        }
        lock.refs++;
    }

    // do pixel transfer if required
    if (transferToGpuNeeded)
        gpu->pushPixels(*bitmap);
    else if (transferFromGpuNeeded)
        gpu->pullPixels(*bitmap);
}


void BitmapContentLock::writeLock(GraphicPipeline* gpu, AbstractBitmap* bitmap, ProcessingTarget target) {
#ifdef BEATMUP_DEBUG
    if (target == ProcessingTarget::GPU)
        DebugAssertion::check(gpu, "Cannot lock a bitmap on GPU: no graphic pipeline instance available");
#endif

    auto it = bitmaps.find(bitmap);
    if (it == bitmaps.end()) {
        // creating a lock descriptor
        LockDescriptor lock{
            false, true,
            target == ProcessingTarget::CPU,
            target == ProcessingTarget::GPU,
            target == ProcessingTarget::CPU,    // data is locked if writing on CPU
            1
        };
        bitmaps.emplace(bitmap, lock);

        // locking pixel data in RAM
        if (lock.isDataLocked)
            bitmap->lockPixelData();
    }

    else {
        // already locked, just increase reference counter
        auto& lock = it->second;

        if (lock.read && !lock.write)
            throw RuntimeError("Cannot a bitmap for writing: it was locked for reading before. Lock it for writing first.");

        // lock data if needed and not yet
        if (!lock.isDataLocked && target == ProcessingTarget::CPU) {
            bitmap->lockPixelData();
            lock.isDataLocked = true;
        }

        lock.gpu |= (target == ProcessingTarget::GPU);
        lock.cpu |= (target == ProcessingTarget::CPU);

        lock.refs++;
    }
}


void BitmapContentLock::unlock(AbstractBitmap* bitmap) {
    auto it = bitmaps.find(bitmap);
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(it != bitmaps.end(), "Trying to unlock a bitmap that is not locked.");
#endif

    auto& lock = it->second;
    lock.refs--;
    if (lock.refs == 0) {
        if (lock.isDataLocked)
            bitmap->unlockPixelData();

        if (lock.write) {
            bitmap->upToDate[ProcessingTarget::CPU] = lock.cpu;
            bitmap->upToDate[ProcessingTarget::GPU] = lock.gpu;
        }

        bitmaps.erase(it);
    }
}


void BitmapContentLock::unlockAll() {
    for (auto it : bitmaps) {
        auto& lock = it.second;
        if (lock.isDataLocked)
            it.first->unlockPixelData();

        if (lock.write) {
            it.first->upToDate[ProcessingTarget::CPU] = lock.cpu;
            it.first->upToDate[ProcessingTarget::GPU] = lock.gpu;
        }
    }

    bitmaps.clear();
}
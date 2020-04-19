#include "bitmap_locker.h"
#include "../exception.h"

using namespace Beatmup;


BitmapLocker::BitmapLocker() {}


BitmapLocker::~BitmapLocker() {
    unlockAll();
}


void BitmapLocker::lock(AbstractBitmap& bitmap, PixelFlow flow) {
    auto container = bitmaps.find(&bitmap);
    if (container == bitmaps.end()) {
        // newbie got
        bitmaps.emplace(&bitmap, std::make_pair<>(flow, 1));
        bitmap.lockContent(flow);
    }
    else {
        // already locked, just increase reference counter
#ifdef BEATMUP_DEBUG
        const PixelFlow origFlow = container->second.first;
        DebugAssertion::check(!(flow | PixelFlow::CpuWrite) || (origFlow | PixelFlow::CpuWrite),
            "Trying to lock a bitmap for writing on GPU, but it is locked for reading.");
        DebugAssertion::check(!(flow | PixelFlow::GpuWrite) || (origFlow | PixelFlow::GpuWrite),
            "Trying to lock a bitmap for writing on CPU, but it is locked for reading.");
#endif
        container->second.second++;
    }
}


void BitmapLocker::unlock(AbstractBitmap& bitmap) {
    auto container = bitmaps.find(&bitmap);
    if (container != bitmaps.end()) {
        auto& entry = container->second;
        entry.second--;
        if (entry.second == 0) {
            bitmap.unlockContent(entry.first);
            bitmaps.erase(container);
        }
    }
}


void BitmapLocker::unlockAll() {
    for (auto& _ : bitmaps)
        _.first->unlockContent(_.second.first);
    bitmaps.clear();
}


bool BitmapLocker::isLocked(AbstractBitmap& bitmap) const {
    return bitmaps.count(&bitmap) > 0;
}

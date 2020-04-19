/*
    Locking multiple bitmaps without risk of locking the same bitmap twice
*/

#pragma once
#include "../bitmap/abstract_bitmap.h"
#include <map>

namespace Beatmup {

    class BitmapLocker {
    private:
        std::map<AbstractBitmap*, std::pair<PixelFlow, int>> bitmaps;
    public:
        BitmapLocker();
        ~BitmapLocker();

        void lock(AbstractBitmap& bitmap, PixelFlow flow);
        void unlock(AbstractBitmap& bitmap);
        void unlockAll();

        bool isLocked(AbstractBitmap& bitmap) const;
    };

}

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

#include "swapper.h"
#include "../context.h"
#include "../gpu/pipeline.h"

using namespace Beatmup;

bool Swapper::processOnGPU(GraphicPipeline& gpu, TaskThread&) {
    BitmapContentLock lock;
    lock.writeLock(&gpu, bitmap, ProcessingTarget::CPU);
    lock.writeLock(&gpu, bitmap, ProcessingTarget::GPU);

    if (fromGpuToCpu)
        gpu.pullPixels(*bitmap);
    else
        gpu.pushPixels(*bitmap);

    lock.unlockAll();
    return true;
}

Swapper::Swapper(bool fromGpuToCpu) : bitmap(nullptr), fromGpuToCpu(fromGpuToCpu)
{}


void Swapper::setBitmap(AbstractBitmap& bitmap) {
    this->bitmap = &bitmap;
}


void Swapper::pullPixels(AbstractBitmap& bitmap) {
    if (!bitmap.isUpToDate(ProcessingTarget::CPU) && bitmap.isUpToDate(ProcessingTarget::GPU)) {
        Swapper me(true);
        me.setBitmap(bitmap);
        bitmap.getContext().performTask(me);
    }
}


void Swapper::pushPixels(AbstractBitmap& bitmap) {
    if (!bitmap.isUpToDate(ProcessingTarget::GPU) && bitmap.isUpToDate(ProcessingTarget::CPU)) {
        Swapper me(false);
        me.setBitmap(bitmap);
        bitmap.getContext().performTask(me);
    }
}

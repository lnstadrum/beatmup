#include "swapper.h"
#include "../context.h"
#include "../gpu/pipeline.h"

using namespace Beatmup;

bool Swapper::processOnGPU(GraphicPipeline& gpu, TaskThread&) {
    bitmap->lockPixels(ProcessingTarget::CPU);
    if (fromGpuToCpu) {
        gpu.fetchPixels(*bitmap);
    }
    else {
        bitmap->lockPixels(ProcessingTarget::GPU);
        gpu.bind(*bitmap, 0, INTERP_NEAREST);
    }
    bitmap->unlockPixels();
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
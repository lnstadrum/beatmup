#include "swapper.h"
#include "../environment.h"
#include "../gpu/pipeline.h"

using namespace Beatmup;

bool Swapper::processOnGPU(GraphicPipeline& gpu, TaskThread&) {
    // grab pixels
    bitmap->lockPixels(ProcessingTarget::CPU);
    gpu.fetchPixels(*bitmap);
    bitmap->unlockPixels();

    return true;
}

Swapper::Swapper() : bitmap(NULL)
{}


void Swapper::setBitmap(AbstractBitmap& bitmap) {
    this->bitmap = &bitmap;
}


void Swapper::grabPixels(AbstractBitmap& bitmap) {
    if (!bitmap.isUpToDate(ProcessingTarget::CPU) && bitmap.isUpToDate(ProcessingTarget::GPU)) {
        Swapper me;
        me.setBitmap(bitmap);
        bitmap.getEnvironment().performTask(me);
    }
}
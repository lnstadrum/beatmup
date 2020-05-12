#include "rendering_context.h"

using namespace Beatmup;

RenderingContext::RenderingContext(GraphicPipeline& gpu, EventListener* eventListener, const float outputWidth, const bool renderingOnScreen) :
    eventListener(eventListener), outputWidth(outputWidth), renderingOnScreen(renderingOnScreen), gpu(gpu)
{
    mapping.setIdentity();
}


void RenderingContext::lockBitmap(AbstractBitmap* bitmap) {
    lockedBitmaps.lock(*bitmap, PixelFlow::GpuRead);
}


void RenderingContext::unlockBitmap(AbstractBitmap* bitmap) {
    lockedBitmaps.unlock(*bitmap);
}


void RenderingContext::blend() {
    gpu.getRenderingPrograms().blend(renderingOnScreen);
}


void RenderingContext::enableProgram(GL::Program& program) {
    gpu.getRenderingPrograms().enableProgram(&gpu, program, true);
}


void RenderingContext::enableProgram(GL::RenderingPrograms::Operation program) {
    gpu.getRenderingPrograms().enableProgram(&gpu, program);
}


GL::Program& RenderingContext::getProgram() {
    return gpu.getRenderingPrograms().getCurrentProgram();
}


void RenderingContext::bindMask(AbstractBitmap& mask) {
    gpu.getRenderingPrograms().bindMask(&gpu, mask);
}

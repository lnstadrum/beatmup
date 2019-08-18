#include "rendering_context.h"

using namespace Beatmup;

RenderingContext::RenderingContext(GraphicPipeline& gpu, EventListener* eventListener, const float outputWidth, const bool renderingOnScreen) :
	gpu(gpu), eventListener(eventListener), cameraFrame(nullptr), outputWidth(outputWidth), renderingOnScreen(renderingOnScreen)
{
	mapping.setIdentity();
}


RenderingContext::~RenderingContext() {
	for (auto it : lockedBitmaps)
		it.first->unlockPixels();
}


void RenderingContext::lockBitmap(BitmapPtr bitmap) {
	if (lockedBitmaps.count(bitmap) == 0) {
		// newbie got
		lockedBitmaps[bitmap] = 1;
		bitmap->lockPixels(ProcessingTarget::GPU);
	}
	else
		// already locked, just increase reference counter
		lockedBitmaps[bitmap]++;
}


void RenderingContext::unlockBitmap(BitmapPtr bitmap) {
	BEATMUP_ASSERT_DEBUG(lockedBitmaps.count(bitmap) != 0);
	int refs = --lockedBitmaps[bitmap];
	if (refs == 0) {
		bitmap->unlockPixels();
		lockedBitmaps.erase(bitmap);
	}
}


void RenderingContext::blend() {
	gpu.getRenderingPrograms().blend(renderingOnScreen);
}


void RenderingContext::enableProgram(GL::Program& program) {
	gpu.getRenderingPrograms().enableProgram(&gpu, program);
}


void RenderingContext::enableProgram(RenderingPrograms::Program program) {
	gpu.getRenderingPrograms().enableProgram(&gpu, program);
}


GL::Program& RenderingContext::getProgram() {
	return gpu.getRenderingPrograms().getCurrentProgram();
}


void RenderingContext::bindMask(AbstractBitmap& mask) {
	gpu.getRenderingPrograms().bindMask(&gpu, mask);
}


GL::TextureHandler* RenderingContext::getCameraFrame() {
	if (!cameraFrame && eventListener)
		eventListener->onCameraFrameRendering(cameraFrame);
	return cameraFrame;
}
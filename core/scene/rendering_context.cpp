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

#include "rendering_context.h"

using namespace Beatmup;

RenderingContext::RenderingContext(GraphicPipeline& gpu, EventListener* eventListener, const ImageResolution& outputResolution, const float outputWidth, const bool renderingOnScreen) :
    eventListener(eventListener), outputResolution(outputResolution), outputWidth(outputWidth), renderingOnScreen(renderingOnScreen), gpu(gpu)
{
    mapping.setIdentity();
}


void RenderingContext::lockBitmap(AbstractBitmap* bitmap) {
    readLock(&gpu, bitmap, ProcessingTarget::GPU);
}


void RenderingContext::unlockBitmap(AbstractBitmap* bitmap) {
    unlock(bitmap);
}


void RenderingContext::blend() {
    gpu.getRenderingPrograms().blend(renderingOnScreen);
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

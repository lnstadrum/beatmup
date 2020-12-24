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

#include "renderer.h"
#include "../geometry.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/interpolation.h"
#include "../bitmap/internal_bitmap.h"
#include "../bitmap/tools.h"
#include "../exception.h"
#include "../gpu/pipeline.h"

#include "../debug.h"

using namespace Beatmup;


/**
    \internal
    Recursive scene rendering instruction
*/
void SceneRenderer::renderLayer(RenderingContext& context, TaskThread& thread, Scene::Layer& layer, const AffineMapping& base, unsigned int recursionLevel) {
    if (recursionLevel >= MAX_RECURSION_LEVEL)
        return;

    switch (layer.getType()) {

    case Scene::Layer::Type::SceneLayer: {
        const Scene& scene = layer.castTo<Scene::SceneLayer>().getScene();
        for (int i = 0; i < scene.getLayerCount() && !thread.isTaskAborted(); ++i) {
            Scene::Layer& l = scene.getLayer(i);
            if (l.isVisible())
                renderLayer(context, thread, l, base * layer.getMapping(), recursionLevel + 1);
        }
    }
    break;

    default:
        context.setMapping(base * layer.getMapping());
        layer.render(context);
        break;
    }
}


const Scene* SceneRenderer::getScene() const {
    return scene;
}


AbstractBitmap* SceneRenderer::getOutput() const {
    return output;
}


void SceneRenderer::setScene(Scene* scene) {
    this->scene = scene;
}


void SceneRenderer::setOutput(AbstractBitmap* output) {
    this->output = output;
}


void SceneRenderer::resetOutput() {
    this->output = nullptr;
}


void SceneRenderer::setOutputMapping(const OutputMapping newMapping) {
    outputMapping = newMapping;
}


SceneRenderer::OutputMapping SceneRenderer::getOutputMapping() const {
    return outputMapping;
}


void SceneRenderer::setOutputReferenceWidth(int newWidth) {
    referenceWidth = newWidth;
}


int SceneRenderer::getOutputReferenceWidth() const {
    return referenceWidth;
}


void SceneRenderer::setBackgroundImage(AbstractBitmap* bitmap) {
    background = bitmap;
}


void SceneRenderer::setOutputPixelsFetching(bool fetch) {
    this->outputPixelsFetching = fetch;
}


bool SceneRenderer::getOutputPixelsFetching() const {
    return outputPixelsFetching;
}


Scene::Layer* SceneRenderer::pickLayer(float x, float y, bool inPixels) const {
    if (scene == nullptr)
        return nullptr;
    if (inPixels) {
        const Point p = outputCoords.getInverse(x / resolution.getWidth(), y / resolution.getHeight());
        return scene->getLayer(p.x, p.y);
    }
    else
        return scene->getLayer(x, y);
}


void SceneRenderer::setRenderingEventListener(RenderingContext::EventListener* eventListener) {
    this->eventListener = eventListener;
}


void SceneRenderer::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    if (eventListener)
        eventListener->onRenderingStart();

    // reset camera frame
    cameraFrame = nullptr;
}


bool SceneRenderer::doRender(GraphicPipeline& gpu, TaskThread& thread) {
    // check if there is the content to render
    if (!scene)
        return true;

    // init rendering context
    resolution = output ? output->getSize() : gpu.getDisplayResolution();
    RenderingContext context(gpu, eventListener,
        resolution,
        referenceWidth > 0 ? referenceWidth : resolution.getWidth(),
        output == nullptr);

    // set output
    if (output) {
        context.writeLock(&gpu, output, ProcessingTarget::GPU);
        gpu.bindOutput(*output);
    }
    else {
        gpu.unbindOutput();
    }

    // background
    if (background) {
        context.lockBitmap(background);
        gpu.getRenderingPrograms().paveBackground(&gpu, *background, output);
    }

    // enable blending
    gpu.switchMode(GraphicPipeline::Mode::RENDERING);

    // compute initial mapping
    outputCoords.setIdentity();
    switch (outputMapping) {
    case FIT_WIDTH_TO_TOP:
        outputCoords.matrix.scale(1.0f, resolution.getAspectRatio());
        break;
    case FIT_WIDTH:
        outputCoords.matrix.scale(1.0f, resolution.getAspectRatio());
        outputCoords.setCenterPosition(Point(0.5f, 0.5f));
        break;
    case FIT_HEIGHT:
        outputCoords.matrix.scale(resolution.getInvAspectRatio(), 1.0f);
        outputCoords.setCenterPosition(Point(0.5f, 0.5f));
        break;
    case STRETCH:
        // identity is okay
        break;
    }

    // go
    for (int i = 0; i < scene->getLayerCount() && !thread.isTaskAborted(); ++i) {
        Scene::Layer& layer = scene->getLayer(i);
        if (layer.isVisible())
            renderLayer(context, thread, layer, outputCoords);
    }

    // finalize output
    if (!thread.isTaskAborted()) {
        if (!output)
            gpu.swapBuffers();
        else if (outputPixelsFetching) {
            context.writeLock(&gpu, output, ProcessingTarget::CPU);
            gpu.pullPixels(*output);
            context.unlock(output);
        }
    }

    // unlock all
    context.unlockAll();
    return true;
}


bool SceneRenderer::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    if (!scene)
        return true;
    Scene::LockGuard lock(scene);

    // go
    return doRender(gpu, thread);
}


bool SceneRenderer::process(TaskThread& thread) {
    throw RuntimeError("GPU is required for rendering");
    return true;
}


SceneRenderer::SceneRenderer():
    scene(nullptr), background(nullptr), output(nullptr),
    outputMapping(FIT_WIDTH_TO_TOP),
    referenceWidth(0),
    outputPixelsFetching(false),
    eventListener(nullptr)
{}


SceneRenderer::~SceneRenderer() {}

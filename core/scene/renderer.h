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

#pragma once
#include "scene.h"
#include "../gpu/gpu_task.h"
#include <map>

namespace Beatmup {
    /**
        AbstractTask rendering a Scene.
        The rendering may be done to a given bitmap or on screen, if the platform supports on-screen rendering.
    */
    class SceneRenderer : public GpuTask {
    public:
        /**
            Scene coordinates to output (screen or bitmap) pixel coordinates mapping
        */
        enum OutputMapping {
            STRETCH = 0,        //!< output viewport covers entirely the scene axis span, aspect ratio is not preserved in general
            FIT_WIDTH_TO_TOP,   //!< width is covered entirely, height is resized to keep aspect ratio, the top borders are aligned
            FIT_WIDTH,          //!< width is covered entirely, height is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
            FIT_HEIGHT          //!< height is covered entirely, width is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
        };

    private:
        Scene* scene;                               //!< content to render
        AbstractBitmap* background;                 //!< used to pave the screen before rendering
        AbstractBitmap* output;                     //!< output bitmap
        OutputMapping outputMapping;                //!< specifies how the scene coordinates [0,1] are mapped to the output (screen or bitmap)
        AffineMapping outputCoords;                 //!< the actual output mapping used during the last rendering
        ImageResolution resolution;                 //!< last rendered resolution
        int referenceWidth;                         //!< value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
        bool outputPixelsFetching;                  //!< if `true`, the output bitmap data is fetched from GPU to CPU RAM every time the rendering is done
        GL::TextureHandler* cameraFrame;            //!< last got camera frame; set to NULL before rendering, then asked from outside through eventListener
        RenderingContext::EventListener* eventListener;

        static const unsigned int
            MAX_RECURSION_LEVEL = 256;
        void renderLayer(RenderingContext& context, TaskThread& thread, Scene::Layer& layer, const AffineMapping& base, unsigned int recursionLevel = 0);
        bool doRender(GraphicPipeline& gpu, TaskThread& thread);

    protected:
        bool process(TaskThread& thread);
        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
    public:
        SceneRenderer();
        ~SceneRenderer();

        /**
            Attaches a bitmap to the renderer output
        */
        void setOutput(AbstractBitmap* bitmap);

        /**
            \return a pointer to output bitmap
        */
        AbstractBitmap* getOutput() const;

        /**
            Removes a bitmap from the renderer output, if any, and switches to on-screen rendering.
            The rendering is done on the display currently connected to the Context running the rendering task.
        */
        void resetOutput();

        const Scene* getScene() const;

        void setScene(Scene* scene);

        /**
            Specifies the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap) pixel coordinates.
            \param mapping      The coordinates mapping
        */
        void setOutputMapping(const OutputMapping mapping);

        /**
            Retrieves the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap) pixel coordinates.
            \return the mapping.
        */
        OutputMapping getOutputMapping() const;

        /**
            Sets a value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture.
            \param newWidth     The new reference width in pixels. If set negative or zero, the actual output image width is used.
        */
        void setOutputReferenceWidth(int newWidth);

        /**
            \return value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture.
        */
        int getOutputReferenceWidth() const;

        /**
            Specifies whether the output image data is pulled from GPU to CPU memory every time the rendering is done.
            This is convenient if the rendered image is an application output result, and is further stored or sent through the network.
            Otherwise, if the image is to be further processed inside %Beatmup, the pixel transfer likely introduces an unnecessary latency and may
            cause FPS drop in real-time rendering.
            Has no effect in on-screen rendering.
            \param[in] fetch    If `true`, pixels are pulled to CPU memory.
        */
        void setOutputPixelsFetching(bool fetch);

        /**
            Reports whether the output bitmap pixels are automatically offloaded from GPU to CPU memory every time the rendering is done.
        */
        bool getOutputPixelsFetching() const;

        /**
            Sets an image to pave the background.
        */
        void setBackgroundImage(AbstractBitmap*);

        /**
            \return the bitmap currently set to fill the background, null if not set.
        */
        inline AbstractBitmap* getBackgroundImage() const { return background; }

        /**
            Retrieves a scene layer visible at a given point, if any.
            In contrast to Scene::getLayer() it takes into account the output mapping.
            "Phantom" layers are ignored.
            \param x            Horizontal coordinate.
            \param y            Vertical coordinate.
            \param inPixels     If `true`, the coordinates are taken in pixels.
            \return the topmost layer at the given position if any, `null` if no layer found.
        */
        Scene::Layer* pickLayer(float x, float y, bool inPixels) const;

        void setRenderingEventListener(RenderingContext::EventListener* eventListener);
    };}

/*
    Scene renderer
*/
#pragma once
#include "scene.h"
#include "../gpu/gpu_task.h"
#include <map>
namespace Beatmup {
    class SceneRenderer : public GpuTask {
    public:
        /**
            Scene coordinates to output (screen or bitmap) mapping
        */
        enum OutputMapping {
            STRETCH = 0,		//!< output viewport covers entirely the scene axis span, aspect ratio is not preserved in general
            FIT_WIDTH_TO_TOP,	//!< width is covered entirely, height is resized to keep aspect ratio, the top borders are aligned
            FIT_WIDTH,			//!< width is covered entirely, height is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
            FIT_HEIGHT			//!< height is covered entirely, width is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
        };
    private:
        Scene* scene;								//!< content to render
        AbstractBitmap* background;					//!< used to pave the screen before rendering
        BitmapPtr output;							//!< output bitmap
        OutputMapping outputMapping;				//!< specfies how the scene coordinates [0,1] are mapped to the output (screen or bitmap)
        AffineMapping outputCoords;					//!< the actual output mapping used during the last rendering
        ImageResolution resolution;					//!< last rendered resolution
        int referenceWidth;							//!< value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
        bool outputPixelsFetching;					//!< if `true`, the output bitmap data is fetched from GPU to CPU RAM every time the rendering is done
        GL::TextureHandler* cameraFrame;			//!< last got camera frame; set to NULL before rendering, then asked from outside through eventListener
        RenderingContext::EventListener* eventListener;
        static const unsigned int
            MAX_RECURSION_LEVEL = 256;
        void renderLayer(RenderingContext& context, TaskThread& thread, Scene::Layer& layer, const AffineMapping& base, unsigned int recursionLevel = 0);
        bool doRender(GraphicPipeline& gpu, TaskThread& thread);
    protected:
        bool process(TaskThread& thread);
        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, bool aborted);
    public:
        SceneRenderer();
        ~SceneRenderer();
        /**
            Attaches a bitmap to the renderer output
        */
        void setOutput(AbstractBitmap& bitmap);
        /**
            \return a pointer to output bitmap
        */
        AbstractBitmap* getOutput() const;
        /**
            Removes a bitmap from the renderer output, if any, and switches to on-screen rendering
        */
        void resetOutput();
        const Scene* getScene() const;
        void setScene(Scene& scene);
        /**
            \brief Specifies the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap).
            \param mapping		The coordinates mapping.
        */
        void setOutputMapping(const OutputMapping mapping);
        /**
            \brief Retrieves the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap).
            \return the mapping.
        */
        OutputMapping getOutputMapping() const;
        /**
            \brief	Sets a value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture.
            \param newWidth		The new reference width in pixels. If set negative or zero, the actual output image width is used.
        */
        void setOutputReferenceWidth(int newWidth);
        /**
            \brief Retrieves value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
        */
        int getOutputReferenceWidth() const;
        /**
            \brief Sets an image to pave the background when rendering to screen.
            \param[in] bitmap		The image to use as the background.
        */
        void setBackgroundImage(AbstractBitmap* bitmap);
        /**
            \brief Specifies whether the output bitmap should be fetched from GPU to CPU memory every time the rendering is done.
            \param[in] fetch	If `true`, pixels are grabbed to CPU memory.
        */
        void setOutputPixelsFetching(bool fetch);
        /**
            \brief Reports whether the output bitmap pixels are automatically offloaded from GPU to CPU memory every time the rendering is done.
        */
        bool getOutputPixelsFetching() const;
        /**
            \brief Searches for a layer at a given position.
            In contrast to Scene::getLayer() it takes into account the output mapping.
            \param x			x coordinate.
            \param y			y coordinate.
            \param normalized	If `true`, the coordinates are normalized to [0..1)*[0..h/w) range.
                                Otherwise they are interpreted in pixels.
            \return the topmost layer at the given position if any, `null` if no layer found.
        */
        Scene::Layer* pickLayer(float x, float y, bool normalized) const;
        void setRenderingEventListener(RenderingContext::EventListener* eventListener);
    };}
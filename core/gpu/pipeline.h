/*
    Low-level graphic pipeline manipulation
*/

#pragma once

#include "../geometry.h"
#include "../gpu/texture_handler.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/pixel_arithmetic.h"
#include "program.h"
#include "../scene/rendering_programs.h"

#include <mutex>


namespace Beatmup {
    class GraphicPipeline {
        friend class GL::TextureHandler;
    private:
        class Impl;
        Impl* impl;
        std::mutex access;

        RenderingPrograms renderingPrograms;

        GraphicPipeline(const GraphicPipeline&) = delete;				//!< disabling copy constructor

    public:
        enum class Limit {
            LOCAL_GROUPS_X,
            LOCAL_GROUPS_Y,
            LOCAL_GROUPS_Z,
            LOCAL_GROUPS_TOTAL,
            SHARED_MEM,
        };

        GraphicPipeline();
        ~GraphicPipeline();

        void lock();

        void unlock();

        void switchDisplay(void* data);

        void swapBuffers();

        /**
            Binds a bitmap to the pipeline output.
            \param[in] bitmap      A bitmap to be filled with pixels on the next render pass.
        */
        void bindOutput(AbstractBitmap& bitmap);

        /**
            Binds a bitmap to the pipeline output.
            \param[in] bitmap      A bitmap to be filled with pixels on the next render pass.
            \param[in] viewport    Output viewport in pixels: only this area of the bitmap will be affected
        */
        void bindOutput(AbstractBitmap& bitmap, const IntRectangle& viewport);

        /**
            Unbinds a bitmap from output and switches to screen
        */
        void unbindOutput();

        ImageResolution getOutputResolution() const;

        void bind(GL::TextureHandler& texture, size_t texUnit, const TextureParam param);
        void bind(GL::TextureHandler& texture, size_t imageUnit, bool read, bool write);

        void fetchPixels(AbstractBitmap& bitmap);

        /**
            Waits until all operations submitted to GPU are finished.
        */
        void flush();

        int getLimit(Limit limit) const;

        /**
            Enables / disables alpha blending.
            \param[in] enable    If `true`, the image is blended on top of what is in the framebuffer according to alpha
                                 channel values. Otherwise its alpha is simply copied.
        */
        void switchAlphaBlending(bool enable);

        inline RenderingPrograms & getRenderingPrograms() { return renderingPrograms; }

        const char* getGpuVendorString() const;
        const char* getGpuRendererString() const;
    };
}

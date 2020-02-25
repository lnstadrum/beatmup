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

      	/**
        	To be called at the end of a rendering pass to ensure a proper state of the output.
        */
        void flush();
        
        void swapBuffers();

        /**
            Binds a texture handler to the pipeline output
        */
        void bindOutput(AbstractBitmap&);

        /**
            Unbinds a bitmap from output and switches to screen
        */
        void unbindOutput();
        
        ImageResolution getOutputResolution() const;

        void bind(GL::TextureHandler& texture, size_t texUnit, const TextureParam param);
        void bind(GL::TextureHandler& texture, size_t imageUnit, bool read, bool write);

        void fetchPixels(AbstractBitmap& bitmap);

        int getLimit(Limit limit) const;

        inline RenderingPrograms & getRenderingPrograms() { return renderingPrograms; }
    };
}
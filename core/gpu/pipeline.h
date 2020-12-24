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
#include "../geometry.h"
#include "../gpu/texture_handler.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/pixel_arithmetic.h"
#include "program.h"
#include "../gpu/rendering_programs.h"


namespace Beatmup {
    /**
        Internal low-level GPU control API.
        Hides OpenGL stuff from intsances of AbstractTask operating with images. A GraphicPipeline instance is likely a singleton accessible from a single thread.
    */
    class GraphicPipeline {
        friend class GL::TextureHandler;
    private:
        class Impl;
        Impl* impl;

        GL::RenderingPrograms* renderingPrograms;

        GraphicPipeline(const GraphicPipeline&) = delete;    // disabling copy constructor

    public:
        /**
            Graphic pipeline working mode setting some common OpenGL switches
        */
        enum class Mode {
            RENDERING,    //!< Textures are images to be blended together to produce another image
            INFERENCE     //!< Textures are feature maps computed in fragment shaders
        };

        /**
            GPU characteristics
        */
        enum class Limit {
            TEXTURE_IMAGE_UNITS,            //!< maximum number of texture units per fragment shader
            FRAGMENT_UNIFORM_VECTORS,       //!< maximum number of 4-dimensional uniform vectors per fragment shader
            LOCAL_GROUPS_X,
            LOCAL_GROUPS_Y,
            LOCAL_GROUPS_Z,
            LOCAL_GROUPS_TOTAL,
            SHARED_MEM,
        };

        static const int
            ATTRIB_VERTEX_COORD = 0,        //!< vertex coordinate attribute index in the VBO
            ATTRIB_TEXTURE_COORD = 1;       //!< texture coordinate attribute index in the VBO

        GraphicPipeline();
        ~GraphicPipeline();

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

        void bindOutput(GL::TextureHandler& texture);

        void bindOutput(GL::handle_t texture);

        /**
            Unbinds a bitmap from output and switches to screen
        */
        void unbindOutput();

        const ImageResolution& getDisplayResolution() const;

        void bind(GL::TextureHandler& texture, size_t texUnit, const TextureParam param);
        void bind(GL::TextureHandler& texture, size_t imageUnit, bool read, bool write);

        /**
            Transfers bitmap pixels from GPU to CPU. The bitmap is assumed locked.
        */
        void pullPixels(AbstractBitmap& bitmap);

        /**
            Transfers bitmap pixels from CPU to GPU. The bitmap is assumed locked.
        */
        void pushPixels(AbstractBitmap& bitmap);

        /**
            Waits until all operations submitted to GPU are finished.
        */
        void flush();

        int getLimit(Limit limit) const;

        /**
            Switches GPU mode
        */
        void switchMode(Mode mode);

        inline GL::RenderingPrograms & getRenderingPrograms() { return *renderingPrograms; }
        inline const GL::VertexShader& getDefaultVertexShader() const { return renderingPrograms->getDefaultVertexShader(this); }

        const char* getGpuVendorString() const;
        const char* getGpuRendererString() const;

        /**
            Specifies texture coordinates for the next rendering pass.
            \param[in] coords       Normalized texture coordinates
        */
        void setTextureCoordinates(const Rectangle& coords);

        /**
            Specifies texture coordinates for the next rendering pass.
            \param[in] leftTop      Top left output image corner texture coordinates
            \param[in] rightTop     Top right output image corner texture coordinates
            \param[in] leftBottom   Bottom left output image corner texture coordinates
            \param[in] rightBottom  Bottom right output image corner texture coordinates
        */
        void setTextureCoordinates(const Point& leftTop, const Point& rightTop, const Point& leftBottom, const Point& rightBottom);

        /**
            Specifies texture coordinates for the next rendering pass.
            \param[in] area         A closed rectangle in pixels of a texture to sample; all its corners are valid sampling
                                    locations.
            \param[in] size         Size in pixels of the input texture
            \param[in] sampling     Number of resulting samples covering the area
        */
        inline void setTextureCoordinates(const Rectangle& area, const IntPoint& size, const IntPoint& sampling) {
            setTextureCoordinates(getTextureCoordinates(area, size, sampling));
        }

        /**
            Computes floating-point texture coordinates for pixel-accurate sampling: a texture gets sampled exactly
            at specified pixel locations.
            \param[in] area         A closed rectangle in pixels of a texture to sample; all its corners are valid sampling
                                    locations.
            \param[in] size         Size in pixels of the input texture
            \param[in] sampling     Number of resulting samples covering the area
        */
        static Rectangle getTextureCoordinates(const Rectangle& area, const IntPoint& size, const IntPoint& sampling);
    };
}

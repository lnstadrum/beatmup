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
#include "../basic_types.h"
#include "../exception.h"
#include "recycle_bin.h"

namespace Beatmup {
    class GraphicPipeline;

    /**
        Parameters of binding a texture to a texture unit on GPU.
    */
  	enum TextureParam {
        INTERP_NEAREST = 0,    //!< nearest neighbor pixel interpolation
        INTERP_LINEAR = 1,     //!< bilinear pixel interpolation
        REPEAT = 2             //!< wrapping the texture by repeating instead of clamping to edge
    };

    namespace GL {
        class TextureHandler : public Beatmup::Object {
            friend class ::Beatmup::GraphicPipeline;

        public:
            /**
                Texture format, specifies how the texture should be interpreted on the shader side
            */
            enum TextureFormat {
                Rx8,
                RGBx8,
                RGBAx8,
                Rx32f,
                RGBx32f,
                RGBAx32f,
                OES_Ext			//!< external EGL image
            };

            static const int TEXTURE_FORMAT_BYTES_PER_PIXEL[];      //!< size of a texel in bytes for different texture formats

            static const char* textureFormatToString(const TextureFormat&);

        protected:
            handle_t textureHandle;

            TextureHandler();

            /**
                Prepares (eventually uploads) texture data on GPU.
                Called only by the context managing thread.
                \param[in] gpu          Graphic pipeline instance
            */
            virtual void prepare(GraphicPipeline& gpu);

            /**
                Forces disposing the texture data, e.g. when it is not used any more
            */
            void invalidate(RecycleBin&);

        public:
            ~TextureHandler();

            /**
                Width of the texture in pixels
            */
            virtual const int getWidth() const = 0;

            /**
                Height of the texture in pixels
            */
            virtual const int getHeight() const = 0;

            /**
                Depth of the texture in pixels
            */
            virtual const int getDepth() const = 0;

            /**
                Aspect ratio of the texture.
            */
            float getAspectRatio() const { return (float)getWidth() / getHeight(); }

            /**
                Inverse of the aspect ratio of the texture.
            */
            float getInvAspectRatio() const { return (float)getHeight() / getWidth(); }

            /**
                Returns the texture format specifying how the shader must interpret the data
            */
            virtual const TextureFormat getTextureFormat() const = 0;

            const bool isFloatingPoint() const {
                const TextureFormat format = getTextureFormat();
                return format == TextureFormat::Rx32f || format == TextureFormat::RGBx32f || format == TextureFormat::RGBAx32f;
            }

            /**
                Returns number of channels containing in the texture
            */
            const int getNumberOfChannels() const;

            /**
                Returns `true` if the texture handle points to a valid texture
            */
            inline bool hasValidHandle() const { return textureHandle > 0; }
        };
    }

    /**
        Exception thrown when texture format does not match any supported format
    */
    class UnsupportedTextureFormat : public Exception {
    public:
        UnsupportedTextureFormat(const GL::TextureHandler::TextureFormat& format) :
            Exception("Input texture format is not supported: %s", GL::TextureHandler::textureFormatToString(format))
        {}
    };
}

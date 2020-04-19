/*
    A texture stored in GPU memory
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

            static const char* textureFormatToString(const TextureFormat&);

        protected:
            glhandle textureHandle;

            TextureHandler();

            /**
                Prepares (eventually uploads) texture data on GPU.
                Called only by the context managing thread.
                \param[in] gpu          Graphic pipeline instance
                \param[in] queryData    If true, the texture data is intended to be used
            */
            virtual void prepare(GraphicPipeline& gpu, bool queryData);

            /**
                Forces disposing the texture data, e.g. when it is not used any more
            */
            void invalidate(RecycleBin&);

        private:
            unsigned int groupIndex;

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

            const int getNumberOfChannels() const;

            /**
                Returns `true` if the texture handle points to a valid texture
            */
            bool hasValidHandle() const { return textureHandle > 0; }

            /**
                Assigns the texture handler to a new group
            */
            void assignToNewGroup();

            /**
                Assigns the texture handler to a group of another texture handler
            */
            void assignGroupFrom(const TextureHandler&);

            /**
                Checks whether the texture handler is assigned to the same group as another texture handler
            */
            bool isOfSameGroup(const TextureHandler&) const;

            /**
                Checks whether the texture handler is assigned to any group
            */
            bool isAssignedToGroup() const;
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
/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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
#include "../gpu/program.h"
#include "../geometry.h"
#include <map>

namespace Beatmup {
    namespace GL {

        /**
            Handles a collection of common rendering programs of predefined types and shared operations among these
            programs.
            Programs compilation and linking is done in a deferred fashion, so that the first rendering pass would
            usually take much more time, but no time is spent on initializing programs that will never be used.
        */
        class RenderingPrograms {
        public:
            /**
                Standard rendering operations
            */
            enum class Operation {
                BLEND,                  //!< default blending of a single image
                MASKED_BLEND,           //!< blending an image through a pixelwise mask
                MASKED_8BIT_BLEND,      //!< blending an image through a 8 bit pixelwise mask
                SHAPED_BLEND,           //!< shaping an image
                BLEND_EXT,              //!< default blending using a texture extension
                MASKED_BLEND_EXT,       //!< blending an image through a pixelwise mask (texture extension)
                MASKED_8BIT_BLEND_EXT,  //!< blending an image through a 8 bit pixelwise mask (texture extension)
                SHAPED_BLEND_EXT,       //!< shaping an image (texture extension)
            };

            static const char
                *VERTEX_COORD_ATTRIB_NAME,      //!< vertex coordinate attribute name in vertex shaders
                *TEXTURE_COORD_ATTRIB_NAME,     //!< texture coordinate attribute name in vertex shaders

                *VERTICAL_FLIP_ID,              //!< Vertical flipping variable name in vertex shader
                *MODELVIEW_MATRIX_ID,           //!< Modelview matrix (mapping input geometry to output) shader variable name in vertex shader
                *TEXTURE_COORDINATES_ID,        //!< Texture coordinates shader variable name in vertex shader

                *DECLARE_TEXTURE_COORDINATES_IN_FRAG;    //!< Declaring texture coordinates in fragment shader.



            /**
                \brief Select and enable a common program.
                If the selected program is not yet ready, it is linked and complied.
                \param[in,out] gpu			A graphic pipeline instance
                \param[in,out] program		The selected common program to use
            */
            void enableProgram(GraphicPipeline* gpu, Operation program);


            /**
                \return the currently used rendering program.
            */
            Program& getCurrentProgram();


            /**
                \brief Binds a mask to a masked rendering program.
                Throws an exception when a mask is about to be bound to a non-masked rendering program.
                \param[in,out] gpu		A graphic pipeline instance
                \param[in,out] mask		The mask bitmap
            */
            void bindMask(GraphicPipeline* gpu, AbstractBitmap& mask);


            /**
                Performs the blending operation
                \param[in] onScreen		If `true`, the rendering is performed on a screen, not into a bitmap.
            */
            void blend(bool onScreen);

            /**
                Fills background with a repeated texture taking 1 pixel of this texture per 1 pixel of the output
                \param[in,out] gpu    A graphic pipeline instance
                \param[in] content    The texture
                \param[in] output     Target texture handler or null if onscreen rendering
            */
            void paveBackground(GraphicPipeline* gpu, TextureHandler& content, GL::TextureHandler* output);


            /**
                \return default blending vertex shader to be used in a user-defined single-image blending program.
            */
            const VertexShader& getDefaultVertexShader(const GraphicPipeline* gpu) const;

            RenderingPrograms(GraphicPipeline* gpu);
            ~RenderingPrograms();

        private:
            class Backend;
            Backend* backend;
            Program* currentGlProgram;
            Operation currentProgram;
            bool maskSetUp;

            VertexShader defaultVertexShader;
            std::map<Operation, RenderingProgram> programs;

            Program& getProgram(const GraphicPipeline* gpu, Operation program);
        };
    }
}

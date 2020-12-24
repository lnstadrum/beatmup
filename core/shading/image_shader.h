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
#include "../gpu/variables_bundle.h"
#include "../gpu/pipeline.h"
#include "../context.h"
#include "../exception.h"
#include <mutex>
#include <string>
#include <map>
namespace Beatmup {
    namespace GL {
        class Program;
    }
    /**
        A GLSL program to process images
    */
    class ImageShader : public GL::VariablesBundle {
        ImageShader(const ImageShader&) = delete;			//!< disabling copying constructor
    private:
        GL::RecycleBin& recycleBin;
        GL::RenderingProgram* program;
        std::string sourceCode;                             //!< last passed fragment shader source code
        bool upToDate;                                      //!< if `true`, the program is up-to-date with respect to the source code
        GL::TextureHandler::TextureFormat inputFormat;      //!< last used input texture format; when changed, the shader is recompiled
        IntRectangle outputClipRect;                        //!< output clip rectangle: only this specified area of the output image will be changed

    public:
        ImageShader(GL::RecycleBin& recycleBin);
        ImageShader(Context& ctx);
        ~ImageShader();

        /**
            Passes new source code to the fragment shader.
            The new source code will be compiled and linked when next rendering occurs.
        */
        void setSourceCode(const std::string& sourceCode);

        /**
            \brief Sets output clipping area.
            Only this specified area of the output bitmap will be changed by executing the shader. This must be called before prepare().
            \param[in] rectangle    The output clipping area in pixels
        */
        void setOutputClipping(const IntRectangle& rectangle);

        /**
            \brief Conducts required preparations for blending. Compiles shaders and links the rendering program if not yet.
            \param gpu        Graphic pipeline instance
            \param input      Shader input image. This is optional, but the shader is considered to have at least one input image if this function is used.
            \param texParam   Input texture parameter
            \param output     Image to write shader output to (optional)
            \param mapping    Geometric transformation to apply when filling output
        */
        void prepare(GraphicPipeline& gpu, GL::TextureHandler* input, const TextureParam texParam, AbstractBitmap* output, const AffineMapping& mapping);
        void prepare(GraphicPipeline& gpu, GL::TextureHandler* input, AbstractBitmap* output);

        /**
            \brief Conducts required preparations for blending. Compiles shaders and links the rendering program if not yet.
            This function is used for shaders having no inputs.
            \param gpu        Graphic pipeline instance
            \param output     Image to write shader output to (optional)
        */
        void prepare(GraphicPipeline& gpu, AbstractBitmap* output);

        /**
            \brief Binds a bunch of texture units to a uniform sampler array variable.
            \param[in] uniformId       The uniform array variable name
            \param[in] startingUnit    First texture unit to be bound to the first element of the array
            \param[in] numUnits        Number of texture units to bind (likely matches the length of the array)
        */
        void bindSamplerArray(const char* uniformId, int startingUnit, int numUnits);

        /**
            \brief Apply the shader to produce an image.
            \param gpu      A graphic pipeline instance
        */
        void process(GraphicPipeline& gpu);

        /**
            Returns `true` if the shader has ressources attached to a given context.
        */
        inline bool usesContext(Context& context) const { return context.getGpuRecycleBin() == &recycleBin; }

        /**
            A virtual input image type defined at shader compile time by ordinary texture
            or OES texture sampler depending on the input bound
        */
        static const std::string INPUT_IMAGE_DECL_TYPE;

        /**
            Shader variable name referring to the input image
        */
        static const std::string INPUT_IMAGE_ID;

        /**
            Shader code header containing necessary declarations
        */
        static const std::string CODE_HEADER;

        /**
            Expection thrown if no shader source is provided
        */
        class NoSource : public Exception {
        public:
            NoSource() : Exception("Layer shader has no source code") {}
        };
    };}

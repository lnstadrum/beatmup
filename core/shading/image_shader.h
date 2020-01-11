/*
    A GLSL program to process images
*/
#pragma once
#include "../gpu/variables_bundle.h"
#include "../gpu/pipeline.h"
#include "../environment.h"
#include "../exception.h"
#include <mutex>
#include <string>
#include <map>
namespace Beatmup {
    namespace GL {
        class Program;
        class VertexShader;
        class FragmentShader;
    }
    /**
        A GLSL program to process images
    */
    class ImageShader : public GL::VariablesBundle {
        ImageShader(const ImageShader&) = delete;			//!< disabling copying constructor
    private:
        Environment& env;
        GL::Program* program;
        GL::FragmentShader* fragmentShader;
        std::string sourceCode;                             //!< last passed fragment shader source code
        GL::TextureHandler::TextureFormat inputFormat;		//!< last used input texture format; when changed, the shader is recompiled
        bool fragmentShaderReady;                           //!< if `true`, shader is ready to go
    public:
        ImageShader(Environment& env);
        ~ImageShader();
        /**
            Passes new source code to the fragment shader.
            The new source code will be compiled and linked when next rendering occurs.
        */
        void setSourceCode(const char* sourceCode);
        void setSourceCode(const std::string& sourceCode) {
            setSourceCode(sourceCode.c_str());
        }
        /**
            \brief Conducts required preparations for the blending. Compiles shaders and links the rendering program if not yet.
            \param gpu        Graphic pipeline instance
            \param input      Shader input image (optional)
            \param output     Image to write shader output to (optional)
            \param mapping    Geometric transformation to apply when filling output
        */
        void prepare(GraphicPipeline& gpu, GL::TextureHandler* input, AbstractBitmap* output, const AffineMapping& mapping);
        /**
            \brief Apply the shader to produce an image.
            \param gpu      A graphic pipeline instance
        */
        void process(GraphicPipeline& gpu);
        /**
            A virtual input image type defined at shader compile time by ordinary texture
            or OES texture sampler depending on the input bound.
        */
        static const std::string INPUT_IMAGE_DECL_TYPE;
        /**
            Shader variable name referring to the input image.
        */
        static const std::string INPUT_IMAGE_ID;
        static const std::string CODE_HEAD;
        /**
            Expection thrown if no shader source is provided
        */
        class NoSource : public Exception {
        public:
            NoSource() : Exception("Layer shader has no source code") {}
        };
        /**
            Exception thrown when the input texture format does not match any supported format
        */
        class UnsupportedInputTextureFormat : public Exception {
        public:
            UnsupportedInputTextureFormat(const GL::TextureHandler::TextureFormat& format):
                Exception("Input texture format is not supported: %s", GL::TextureHandler::textureFormatToString(format))
            {}
        };
    };}
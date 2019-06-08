/*
	A thread-safe wrapping of GLSL program
*/
#pragma once
#include "../gpu/variables_bundle.h"
#include "../gpu/pipeline.h"
#include "../environment.h"
#include "../exception.h"

#include <mutex>
#include <string>

namespace Beatmup {
	namespace GL {
		class Program;
		class VertexShader;
		class FragmentShader;
	}

	/**
		A GLSL shading program used in a layer to render things.
	*/
	class LayerShader : public GL::VariablesBundle {
		LayerShader(const LayerShader&) = delete;			//!< disabling copying constructor

	private:
		Environment& env;
		GL::Program* program;
		GL::FragmentShader* fragmentShader;
		GL::TextureHandler::TextureFormat inputFormat;		//!< last used input texture format; when changed, the shader is recompiled
		std::string sourceCode;								//!< last passed fragment shader source code
		bool fragmentShaderReady;							//!< if `true`, shader is ready to go

	public:
		LayerShader(Environment& env);
		~LayerShader();

		/**
			Passes new source code to the fragment shader.
			The new source code will be compiled and linked when next rendering occurs.
		*/
		void setSourceCode(const char* sourceCode);

		/**
			\internal
			\brief Conducts required preparations for the blending. Compiles shaders and links the rendering program if not yet.
			\param gpu			Graphic pipeline instance
			\param image		Image to blend, may be null
		*/
		void prepare(GraphicPipeline& gpu, GL::TextureHandler* image, const AffineMapping& mapping);
		
		/**
			A preprocessor directive for the fragment shader code to be replaced by an appropriate sampler (ordinary or extension)
		*/
		static const std::string BEATMUP_INPUT_IMAGE_PREPROCESSOR_DIRECTIVE;

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
			UnsupportedInputTextureFormat(GL::TextureHandler::TextureFormat& format):
				Exception("Input texture format is not supported: %s", GL::TextureHandler::textureFormatToString(format))
			{}
		};
	};
}
/*
	OpenGL compute shader/program
*/

#pragma once

#include "program.h"
#include "pipeline.h"
#include "../bitmap/abstract_bitmap.h"

namespace Beatmup {
	namespace GL {
		/**
			GLSL compute program.
			The compute shader is managed inside the class.
		*/
		class ComputeProgram : public AbstractProgram {
		private:
			class Shader : public GL::Shader {
				friend class ComputeProgram;
			public:
				Shader(const GraphicPipeline& gpu);
			};

			Shader shader;

			void link(const GraphicPipeline& gpu);
		public:
			ComputeProgram(const GraphicPipeline& gpu);
			ComputeProgram(const GraphicPipeline& gpu, const char* source);

			void make(const GraphicPipeline& gpu, const char* source);
			void make(const GraphicPipeline& gpu, const std::string& source);
			void dispatch(const GraphicPipeline& gpu, msize w, msize h, msize d) const;
		};
	}
}
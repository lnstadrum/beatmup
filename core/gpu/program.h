/**
	GLSL program
 */

#pragma once
#include "../bitmap/abstract_bitmap.h"
#include "../utils/chunkfile.h"
#include "../basic_types.h"
#include "storage_buffer.h"

namespace Beatmup {
	namespace GL {
	
		class Program;

		class Shader {
			friend class Program;
			Shader(const Shader&) = delete;		//!< disabling copying constructor
		private:
			glhandle handle;
			uint32_t type;
		protected:
			Shader(const GraphicPipeline& gpu, const uint32_t type);
			inline glhandle getHandle() const { return handle; }
			void assertCompiled() const;
		public:
			~Shader();
			void compile(const GraphicPipeline& gpu, const char* source);
			void compile(const GraphicPipeline& gpu, const std::string&);
		};


		class VertexShader : public Shader {
		public:
			VertexShader(const GraphicPipeline& gpu);
			VertexShader(const GraphicPipeline& gpu, const std::string& source) : VertexShader(gpu) { compile(gpu, source.c_str()); }
		};


		class FragmentShader : public Shader {
		public:
			FragmentShader(const GraphicPipeline& gpu);
			FragmentShader(const GraphicPipeline& gpu, const std::string& source) : FragmentShader(gpu) { compile(gpu, source.c_str()); }
		};


		class AtomicCounter {
			friend class AbstractProgram;
		private:
			glhandle handle;
		public:
			AtomicCounter(const GraphicPipeline& gpu);
			~AtomicCounter();

			void set(unsigned int value);
		};


		/**
			Basic GLSL program.
			Shaders are controlled by the user.
		*/
		class AbstractProgram {
		private:
			glhandle handle;
			AbstractProgram(const AbstractProgram&) = delete;		//!< disabling copying constructor

		protected:
			inline glhandle getHandle() const { return handle; }		
			void assertLinked() const;

		public:
			AbstractProgram(const GraphicPipeline& gpu);
			~AbstractProgram();

			void enable(const GraphicPipeline& gpu);

			Chunk* getBinary() const;
			void loadBinary(const Chunk& binary);

			glhandle getUniformLocation(const char* name);
			glhandle getAttribLocation(const char* name);

			/**
				Assignes a value to a specified integer variable in the program
				\param name			the variable name
				\param value		the value to assign
				\param safe			if `true` check if the target variable exists before assigning
			*/
			void setInteger(const char* name, const int value, bool safe = false);

			/**
				Assignes a value to a specified floating poitn variable in the program
				\param name			the variable name
				\param value		the value to assign
				\param safe			if `true` check if the target variable exists before assigning
			*/
			void setFloat(const char* name, const float value, bool safe = false);

			void setVector3(const char* name, const float x, const float y, const float z);
			void setVector4(const char* name, const float x, const float y, const float z, const float w);
			void setMatrix2(const char* name, const Matrix2& mat);		
			void setMatrix3(const char* name, const Matrix2& mat, const Point& pos);
			void setMatrix3(const char* name, const AffineMapping& mapping);

			void bindSampler(GraphicPipeline& gpu, GL::TextureHandler& image, int unit);
			void bindSampler(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId);
			void bindImage(GraphicPipeline& gpu, GL::TextureHandler& image, int unit, bool read, bool write);
			void bindImage(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, bool read, bool write);

			void bindAtomicCounter(GraphicPipeline& gpu, AtomicCounter& counter, int unit);
		};


		class Program : public AbstractProgram {
		private:
			const VertexShader* vertexShader;
			const FragmentShader* fragmentShader;
		public:
			Program(const GraphicPipeline& gpu);
			Program(const GraphicPipeline& gpu, const VertexShader&, const FragmentShader&);

			void link(const VertexShader&, const FragmentShader&);
			void relink(const VertexShader&);
			void relink(const FragmentShader&);

			const VertexShader* getVertexShader() const;
			const FragmentShader* getFragmentShader() const;
		};
	}
}
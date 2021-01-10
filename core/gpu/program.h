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
#include "../bitmap/abstract_bitmap.h"
#include "../utils/chunkfile.h"
#include "../basic_types.h"
#include "storage_buffer.h"
#include <map>

#ifdef BEATMUP_OPENGLVERSION_GLES20
#define BEATMUP_SHADER_HEADER_VERSION "#version 100\n"
#elif BEATMUP_OPENGLVERSION_GLES31
#define BEATMUP_SHADER_HEADER_VERSION ""
#else
#define BEATMUP_SHADER_HEADER_VERSION "#version 430\n"
#endif

#define BEATMUP_SHADER_CODE(...) #__VA_ARGS__
#define BEATMUP_SHADER_CODE_V(...) BEATMUP_SHADER_HEADER_VERSION BEATMUP_SHADER_CODE(__VA_ARGS__)

namespace Beatmup {
    namespace GL {
        class Program;

        /**
            GLSL shader base class
        */
        class Shader {
            friend class Program;
            Shader(const Shader&) = delete;		//!< disabling copying constructor
        private:
            handle_t handle;
            uint32_t type;
        protected:
            Shader(const GraphicPipeline& gpu, const uint32_t type);
            inline handle_t getHandle() const { return handle; }
            inline uint32_t getType() const { return type; }
        public:
            ~Shader();
            void compile(const GraphicPipeline& gpu, const char* source);
            void compile(const GraphicPipeline& gpu, const std::string&);
        };

        /**
            GLSL vertex shader
        */
        class VertexShader : public Shader {
        public:
            VertexShader(const GraphicPipeline& gpu);
            VertexShader(const GraphicPipeline& gpu, const std::string& source) : VertexShader(gpu) { compile(gpu, source.c_str()); }
        };

        /**
            GLSL fragment shader
        */
        class FragmentShader : public Shader {
        public:
            FragmentShader(const GraphicPipeline& gpu);
            FragmentShader(const GraphicPipeline& gpu, const std::string& source) : FragmentShader(gpu) { compile(gpu, source.c_str()); }
        };

        class AtomicCounter {
            friend class AbstractProgram;
        private:
            handle_t handle;
        public:
            AtomicCounter(const GraphicPipeline& gpu);
            ~AtomicCounter();
            void set(unsigned int value);
        };

        /**
            Basic GLSL program.
            Shaders are controlled by the user.
        */
        class AbstractProgram : public GL::RecycleBin::Item {
        private:
            std::map<std::string, handle_t> uniformsCache, attribsCache;
            handle_t handle;
            AbstractProgram(const AbstractProgram&) = delete;		//!< disabling copying constructor

        protected:
            inline handle_t getHandle() const { return handle; }
            void assertLinked() const;
            void clearCaches();

        public:
            AbstractProgram(const GraphicPipeline& gpu);
            ~AbstractProgram();
            void enable(const GraphicPipeline& gpu);

#ifndef BEATMUP_OPENGLVERSION_GLES20
            Chunk* getBinary() const;
            void loadBinary(const Chunk& binary);
#endif

            /**
                Retrieves uniform variable location by its name.
                May be slow.
                \param[in] name     The variable name
            */
            handle_t findUniformLocation(const char* name);

            /**
                Retrieves attribute location by its name.
                May be slow.
                \param[in] name     The attribute name
            */
            handle_t findAttribLocation(const char* name);

            /**
                Retrieves uniform variable location by its name.
                Uses chache. Faster when called more than once.
                \param[in] name     The variable name
            */
            handle_t getUniformLocation(const std::string& name);


            /**
                Retrieves attribute location by its name.
                Uses chache. Faster when called more than once.
                \param[in] name     The attribute name
            */
            handle_t getAttribLocation(const std::string& name);

            /**
                Assigns a value to a specific integer variable in the program.
                \param name			the variable name
                \param value		the value to assign
                \param safe			if `true` check if the target variable exists before assigning
            */
            void setInteger(const std::string& name, const int value, bool safe = false);
            void setUnsignedInteger(const std::string& name, const unsigned int value, bool safe = false);

            /**
                Assigns a value to a specific floating point variable in the program.
                \param name			the variable name
                \param value		the value to assign
                \param safe			if `true` check if the target variable exists before assigning
            */
            void setFloat(const std::string& name, const float value, bool safe = false);

            void setVector2(const std::string& name, const float x, const float y);
            void setVector3(const std::string& name, const float x, const float y, const float z);
            void setVector4(const std::string& name, const float x, const float y, const float z, const float w);
            void setVector4(const std::string& name, const color4i& color, const float outRange = 1.0f);

            void setMatrix2(const std::string& name, const Matrix2& mat);
            void setMatrix3(const std::string& name, const Matrix2& mat, const Point& pos);
            void setMatrix3(const std::string& name, const AffineMapping& mapping);

            void setIntegerArray(const std::string& name, const int* values, const int length);
            void setIntegerArray(const std::string& name, const int firstValue, const int length);

            void setFloatArray(const std::string& name, const float* values, const int length);
            void setVec2Array(const std::string& name, const float* xy, const int length);
            void setVec4Array(const std::string& name, const float* xyzw, const int length);

            void bindSampler(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, TextureParam param);
            void bindImage(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, bool read, bool write);

#ifndef BEATMUP_OPENGLVERSION_GLES20
            void bindAtomicCounter(GraphicPipeline& gpu, AtomicCounter& counter, int unit);
#endif
        };

        /**
            Regular OpenGL program
        */
        class Program : public AbstractProgram {
        public:
            Program(const GraphicPipeline& gpu);
            Program(const GraphicPipeline& gpu, const VertexShader&, const FragmentShader&);
            void link(const VertexShader&, const FragmentShader&);
        };

        /**
            GLSL program to render images
            Makes use of default vertex attributes to pass the texture coordinates and the image position to the GPU
        */
        class RenderingProgram : public Program {
        public:
            RenderingProgram(const GraphicPipeline& gpu, const FragmentShader&);
            RenderingProgram(const GraphicPipeline& gpu, const VertexShader&, const FragmentShader&);
            void link(const GraphicPipeline& gpu, const FragmentShader&);
            void blend(bool onScreen);
            void blend();
        };
    }
}

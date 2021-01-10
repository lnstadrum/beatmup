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

#include "program.h"
#include "pipeline.h"
#include "bgl.h"

using namespace Beatmup;
using namespace GL;


Shader::Shader(const GraphicPipeline& gpu, const uint32_t type) : type(type) {
    handle = glCreateShader(type);
}


Shader::~Shader() {
    glDeleteShader(handle);
}


void Shader::compile(const GraphicPipeline& gpu, const char* source) {
    glShaderSource(handle, 1, &source, 0);
    glCompileShader(handle);

    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
        return;
    GLint logLength;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar* log = (GLchar*)malloc(logLength);
        glGetShaderInfoLog(handle, logLength, &logLength, log);
        std::string msg((char*)log);
        free(log);
#ifdef BEATMUP_DEBUG
        msg = msg + "\n" + source;
#endif
        throw GL::GLException(msg.c_str());
    }
    else
        throw GL::GLException("Shader compilation failed (no log).");
}


void Shader::compile(const GraphicPipeline& gpu, const std::string& src) {
    compile(gpu, src.c_str());
}


VertexShader::VertexShader(const GraphicPipeline& gpu) : Shader(gpu, GL_VERTEX_SHADER) {
}


FragmentShader::FragmentShader(const GraphicPipeline& gpu) : Shader(gpu, GL_FRAGMENT_SHADER) {
}


#ifndef BEATMUP_OPENGLVERSION_GLES20
AtomicCounter::AtomicCounter(const GraphicPipeline& gpu) {
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, handle);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_STATIC_DRAW);
    GL::GLException::check("creating atomic counter");
}


AtomicCounter::~AtomicCounter() {
    glDeleteBuffers(1, &handle);
}


void AtomicCounter::set(unsigned int value) {
    GLuint *data;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, handle);
    data = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    data[0] = value;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    GL::GLException::check("setting atomic counter value");
}
#endif


AbstractProgram::AbstractProgram(const GraphicPipeline& gpu) {
    handle = glCreateProgram();
}


AbstractProgram::~AbstractProgram() {
    glDeleteProgram(handle);
}


void AbstractProgram::assertLinked() const {
    GLint status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if (status == GL_TRUE)
        return;
    GLint logLength;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar* log = (GLchar*)malloc(logLength);
        glGetProgramInfoLog(handle, logLength, &logLength, log);
        throw GL::GLException((char*)log);
        free(log);
    }
    else
        throw GL::GLException("Program linking failed (no log).");
}


void AbstractProgram::clearCaches() {
    uniformsCache.clear();
    attribsCache.clear();
}


void AbstractProgram::enable(const GraphicPipeline& gpu) {
    glUseProgram(handle);
    GL::GLException::check("enabling a program");
}


#ifndef BEATMUP_OPENGLVERSION_GLES20
Chunk* AbstractProgram::getBinary() const {
    GLsizei length;
    glGetProgramiv(handle, GL_PROGRAM_BINARY_LENGTH, &length);
    GL::GLException::check("querying program size");

    // setting up a chunk: first sizeof(GLenum) bytes store the binary format
    Chunk* result = new Chunk(sizeof(GLenum) + (size_t)length);
    glGetProgramBinary(handle, length, nullptr, result->ptr<GLenum>(0), result->ptr<GLenum>(1));
    GL::GLException::check("getting binary");
    return result;
}


void AbstractProgram::loadBinary(const Chunk& binary) {
    // convention: first sizeof(GLenum) bytes store the binary format
    glProgramBinary(handle, binary.at<GLenum>(0), binary.ptr<GLenum>(1), binary.size() - sizeof(GLenum));
    GL::GLException::check("loading program binary");
}
#endif


GL::handle_t AbstractProgram::findUniformLocation(const char* name) {
    return glGetUniformLocation(handle, name);
}


GL::handle_t AbstractProgram::findAttribLocation(const char* name) {
    return glGetAttribLocation(handle, name);
}


GL::handle_t AbstractProgram::getUniformLocation(const std::string& name) {
    auto it = uniformsCache.find(name);
    if (it != uniformsCache.end())
        return it->second;
    handle_t location = glGetUniformLocation(handle, name.c_str());
    uniformsCache[name] = location;
    return location;
}


GL::handle_t AbstractProgram::getAttribLocation(const std::string& name) {
    auto it = attribsCache.find(name);
    if (it != attribsCache.end())
        return it->second;
    handle_t location = glGetAttribLocation(handle, name.c_str());
    attribsCache[name] = location;
    return location;
}


void AbstractProgram::setInteger(const std::string& name, const int value, bool safe) {
    if (safe) {
        GLint location = getUniformLocation(name);
        if (location == -1)
            return;
        glUniform1i(location, (GLint)value);
    }
    else
        glUniform1i(getUniformLocation(name), (GLint)value);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting integer in program");
#endif
}


void AbstractProgram::setUnsignedInteger(const std::string& name, const unsigned int value, bool safe) {
    if (safe) {
        GLint location = getUniformLocation(name);
        if (location == -1)
            return;
#ifdef BEATMUP_OPENGLVERSION_GLES20
        glUniform1i(location, (GLint)value);
#else
        glUniform1ui(location, (GLuint)value);
#endif
    }
    else {
#ifdef BEATMUP_OPENGLVERSION_GLES20
        glUniform1i(getUniformLocation(name), (GLint)value);
#else
        glUniform1ui(getUniformLocation(name), (GLuint)value);
#endif
    }
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting integer in program");
#endif
}


void AbstractProgram::setFloat(const std::string& name, const float value, bool safe) {
    if (safe) {
        GLint location = getUniformLocation(name);
        if (location == -1)
            return;
        glUniform1f(location, (GLfloat)value);
    }
    else
        glUniform1f(getUniformLocation(name), (GLfloat)value);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting float in program");
#endif
}


void AbstractProgram::setVector2(const std::string& name, const float x, const float y) {
    glUniform2f(getUniformLocation(name), x, y);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting vector 2 in program");
#endif
}


void AbstractProgram::setVector3(const std::string& name, const float x, const float y, const float z) {
    glUniform3f(getUniformLocation(name), x, y, z);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting vector 3 in program");
#endif
}


void AbstractProgram::setVector4(const std::string& name, const float x, const float y, const float z, const float w) {
    glUniform4f(getUniformLocation(name), x, y, z, w);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting vector 4 in program");
#endif
}


void AbstractProgram::setVector4(const std::string& name, const color4i& color, const float outRange) {
    const float scale = outRange / 255;
    setVector4(name, scale * color.r, scale * color.g, scale * color.b, scale * color.a);
}


void AbstractProgram::setMatrix2(const std::string& name, const Matrix2& mat) {
    GLfloat m[4] = { 1, 0, 0, 1 };
    mat.getElements(m[0], m[2], m[1], m[3]);
    glUniformMatrix2fv(getUniformLocation(name), 1, false, m);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting matrix 2x2 in program");
#endif
}


void AbstractProgram::setMatrix3(const std::string& name, const Matrix2& mat, const Point& pos) {
    GLfloat m[9] = { 1, 0, 0, 0, 1, 0, pos.x, pos.y, 1 };
    mat.getElements(m[0], m[3], m[1], m[4]);
    glUniformMatrix3fv(getUniformLocation(name), 1, false, m);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting matrix 3x3 in program");
#endif
}


void AbstractProgram::setMatrix3(const std::string& name, const AffineMapping& mapping) {
    setMatrix3(name, mapping.matrix, mapping.position);
}


void AbstractProgram::setIntegerArray(const std::string& name, const int* values, const int length) {
    if (sizeof(GLint) != sizeof(int)) {
        GLint* convValues = new GLint[length];
        for (int i = 0; i < length; ++i)
            convValues[i] = values[i];
        glUniform1iv(getUniformLocation(name), length, convValues);
        delete[] convValues;
    }
    else {
        glUniform1iv(getUniformLocation(name), length, values);
    }
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting integer array in program");
#endif
}


void AbstractProgram::setIntegerArray(const std::string& name, const int firstValue, const int length) {
    static const int STORAGE_LEN = 8;
    GLint storage[STORAGE_LEN];
    const bool alloc = length > STORAGE_LEN;
    GLint* values = alloc ? new GLint[length] : storage;
    for (int i = 0; i < length; ++i)
        values[i] = firstValue + i;
    glUniform1iv(getUniformLocation(name), length, values);
    if (alloc)
        delete[] values;
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting integer array in program");
#endif
}


void AbstractProgram::setFloatArray(const std::string& name, const float* values, const int length) {
    glUniform1fv(getUniformLocation(name), length, values);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting float array in program");
#endif
}


void AbstractProgram::setVec2Array(const std::string& name, const float* xy, const int length) {
    glUniform2fv(getUniformLocation(name), length, xy);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting vec2 array in program");
#endif
}


void AbstractProgram::setVec4Array(const std::string& name, const float* xyzw, const int length) {
    glUniform4fv(getUniformLocation(name), length, xyzw);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting vec4 array in program");
#endif
}


void AbstractProgram::bindSampler(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, TextureParam param) {
    handle_t uniform = getUniformLocation(uniformId);
    GLint unit;
    glGetUniformiv(getHandle(), uniform, &unit);
    GL::GLException::check("binding sampler in program");
    gpu.bind(image, unit, param);
}


void AbstractProgram::bindImage(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, bool read, bool write) {
    handle_t uniform = getUniformLocation(uniformId);
    GLint unit;
    glGetUniformiv(getHandle(), uniform, &unit);
    GL::GLException::check("binding image in program");
    gpu.bind(image, unit, read, write);
}


#ifndef BEATMUP_OPENGLVERSION_GLES20
void AbstractProgram::bindAtomicCounter(GraphicPipeline& gpu, AtomicCounter& counter, int unit) {
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, unit, counter.handle);
    GL::GLException::check("binding atomic counter");
}
#endif


Program::Program(const GraphicPipeline& gpu) : AbstractProgram(gpu) {}


Program::Program(const GraphicPipeline& gpu, const VertexShader& vertex, const FragmentShader& fragment):
    AbstractProgram(gpu)
{
    link(vertex, fragment);
}


void Program::link(const VertexShader& vertexShader, const FragmentShader& fragmentShader) {
    glAttachShader(getHandle(), vertexShader.handle);
    glAttachShader(getHandle(), fragmentShader.handle);
    glLinkProgram(getHandle());
    glDetachShader(getHandle(), vertexShader.handle);
    glDetachShader(getHandle(), fragmentShader.handle);
    assertLinked();
    GL::GLException::check("program linking");
    clearCaches();
}


RenderingProgram::RenderingProgram(const GraphicPipeline& gpu, const FragmentShader& fragmentShader):
    RenderingProgram(gpu, gpu.getDefaultVertexShader(), fragmentShader)
{}


RenderingProgram::RenderingProgram(const GraphicPipeline& gpu, const VertexShader& vertexShader, const FragmentShader& fragmentShader):
    Program(gpu)
{
    glBindAttribLocation(getHandle(), GraphicPipeline::ATTRIB_TEXTURE_COORD, RenderingPrograms::TEXTURE_COORD_ATTRIB_NAME);
    glBindAttribLocation(getHandle(), GraphicPipeline::ATTRIB_VERTEX_COORD,  RenderingPrograms::VERTEX_COORD_ATTRIB_NAME);

    // link
    Program::link(vertexShader, fragmentShader);

    // setting common stuff
    enable(gpu);
    setMatrix3(RenderingPrograms::MODELVIEW_MATRIX_ID, AffineMapping::IDENTITY);
    setInteger(RenderingPrograms::VERTICAL_FLIP_ID, 1);
}


void RenderingProgram::link(const GraphicPipeline& gpu, const FragmentShader& fragmentShader) {
    Program::link(gpu.getDefaultVertexShader(), fragmentShader);
}


void RenderingProgram::blend(bool onScreen) {
    setInteger(RenderingPrograms::VERTICAL_FLIP_ID, onScreen ? 0 : 1);
    blend();
}


void RenderingProgram::blend() {
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

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


void Shader::assertCompiled() const {
    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
        return;
    GLint logLength;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar* log = (GLchar*)malloc(logLength);
        glGetShaderInfoLog(handle, logLength, &logLength, log);
        throw GL::GLException((char*)log);
        free(log);
    }
    else
        throw GL::GLException("Shader compilation failed (no log).");
}


void Shader::compile(const GraphicPipeline& gpu, const char* source) {
    glShaderSource(handle, 1, &source, 0);
    glCompileShader(handle);
    assertCompiled();
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


void AbstractProgram::enable(const GraphicPipeline& gpu) {
    glUseProgram(handle);
    GL::GLException::check("switching on a program");
}


#ifndef BEATMUP_OPENGLVERSION_GLES20
Chunk* AbstractProgram::getBinary() const {
    GLsizei length;
    glGetProgramiv(handle, GL_PROGRAM_BINARY_LENGTH, &length);
    GL::GLException::check("querying program size");
    
    // setting up a chunk: first sizeof(GLenum) bytes store the binary format
    Chunk* result = new Chunk(sizeof(GLenum) + (size_t)length);
    glGetProgramBinary(handle, length, nullptr, result->at<GLenum>(0), result->at<GLenum>(1));
    GL::GLException::check("getting binary");
    return result;
}


void AbstractProgram::loadBinary(const Chunk& binary) {
    // convention: first sizeof(GLenum) bytes store the binary format
    glProgramBinary(handle, *(binary.at<GLenum>(0)), binary.at<GLenum>(1), binary.size() - sizeof(GLenum));
    GL::GLException::check("loading program binary");
}
#endif


glhandle AbstractProgram::getUniformLocation(const char* name) {
    return glGetUniformLocation(handle, name);
}


glhandle AbstractProgram::getAttribLocation(const char* name) {
    return glGetAttribLocation(handle, name);
}


void AbstractProgram::setInteger(const char* name, const int value, bool safe) {
    if (safe) {
        GLint location = glGetUniformLocation(handle, name);
        if (location == -1)
            return;
        glUniform1i(location, (GLint)value);
    }
    else
        glUniform1i(glGetUniformLocation(handle, name), (GLint)value);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting shader integer");
#endif
}


void AbstractProgram::setFloat(const char* name, const float value, bool safe) {
    if (safe) {
        GLint location = glGetUniformLocation(handle, name);
        if (location == -1)
            return;
        glUniform1f(location, (GLfloat)value);
    }
    else
        glUniform1f(glGetUniformLocation(handle, name), (GLfloat)value);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting shader float");
#endif
}


void AbstractProgram::setVector2(const char* name, const float x, const float y) {
    glUniform2f(glGetUniformLocation(handle, name), x, y);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting shader vector 2");
#endif
}


void AbstractProgram::setVector3(const char* name, const float x, const float y, const float z) {
    glUniform3f(glGetUniformLocation(handle, name), x, y, z);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting shader vector 3");
#endif
}


void AbstractProgram::setVector4(const char* name, const float x, const float y, const float z, const float w) {
    glUniform4f(glGetUniformLocation(handle, name), x, y, z, w);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting shader vector 4");
#endif
}


void AbstractProgram::setVector4(const char* name, const color4i& color, const float outRange) {
    const float scale = outRange / 255;
    setVector4(name, scale * color.r, scale * color.g, scale * color.b, scale * color.a);
}


void AbstractProgram::setMatrix2(const char* name, const Matrix2& mat) {
    GLfloat m[4] = { 1, 0, 0, 1 };
    mat.getElements(m[0], m[2], m[1], m[3]);
    glUniformMatrix2fv(glGetUniformLocation(handle, name), 1, false, m);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("seting shader matrix 2x2");
#endif
}


void AbstractProgram::setMatrix3(const char* name, const Matrix2& mat, const Point& pos) {
    GLfloat m[9] = { 1, 0, 0, 0, 1, 0, pos.x, pos.y, 1 };
    mat.getElements(m[0], m[3], m[1], m[4]);
    glUniformMatrix3fv(glGetUniformLocation(handle, name), 1, false, m);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("setting shader matrix 3x3");
#endif
}


void AbstractProgram::setMatrix3(const char* name, const AffineMapping& mapping) {
    setMatrix3(name, mapping.matrix, mapping.position);
}


void AbstractProgram::bindSampler(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, TextureParam param) {
    glhandle uniform = getUniformLocation(uniformId);
    GLint unit;
    glGetUniformiv(getHandle(), uniform, &unit);
    GL::GLException::check("binding sampler in program");
    gpu.bind(image, unit, param);
}


void AbstractProgram::bindImage(GraphicPipeline& gpu, GL::TextureHandler& image, const char* uniformId, bool read, bool write) {
    glhandle uniform = getUniformLocation(uniformId);
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


Program::Program(const GraphicPipeline& gpu, const VertexShader& vertex, const FragmentShader& fragment) : AbstractProgram(gpu) {
    link(vertex, fragment);
}


void Program::link(const VertexShader& vertexShader, const FragmentShader& fragmentShader) {
    this->vertexShader = &vertexShader;
    this->fragmentShader = &fragmentShader;
    glAttachShader(getHandle(), vertexShader.handle);
    glAttachShader(getHandle(), fragmentShader.handle);
    glLinkProgram(getHandle());
    assertLinked();
    GL::GLException::check("program linking");
}


void Program::relink(const VertexShader& vertexShader) {
    this->vertexShader = &vertexShader;
    glAttachShader(getHandle(), vertexShader.handle);
    glLinkProgram(getHandle());
    assertLinked();
}


void Program::relink(const FragmentShader& fragmentShader) {
    this->fragmentShader = &fragmentShader;
    glAttachShader(getHandle(), fragmentShader.handle);
    glLinkProgram(getHandle());
    assertLinked();
}


const VertexShader* Program::getVertexShader() const {
    return vertexShader;
}


const FragmentShader* Program::getFragmentShader() const {
    return fragmentShader;
}
#include "compute_program.h"
#include "bgl.h"

#ifdef BEATMUP_OPENGLVERSION_GLES20
#error Insufficient OpenGL ES version: compute shaders are not available in OpenGL ES 2.0.
#endif

using namespace Beatmup;
using namespace GL;

ComputeProgram::Shader::Shader(const GraphicPipeline& gpu) : GL::Shader(gpu, GL_COMPUTE_SHADER) {}

ComputeProgram::ComputeProgram(const GraphicPipeline& gpu) : AbstractProgram(gpu), shader(gpu) {}

ComputeProgram::ComputeProgram(const GraphicPipeline& gpu, const char* source): ComputeProgram(gpu) {
    make(gpu, source);
}

void ComputeProgram::link(const GraphicPipeline& gpu) {
    glAttachShader(getHandle(), shader.getHandle());
    glLinkProgram(getHandle());
    assertLinked();
}


void ComputeProgram::make(const GraphicPipeline& gpu, const char* source) {
    shader.compile(gpu, source);
    link(gpu);
}


void ComputeProgram::make(const GraphicPipeline& gpu, const std::string& source) {
    make(gpu, source.c_str());
}


void ComputeProgram::dispatch(const GraphicPipeline& gpu, msize w, msize h, msize d) const {
    glDispatchCompute(w, h, d);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("dispatching compute");
#endif
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("memory barrier");
#endif
    glFinish();	// fixme: this helps my Radeon not to crash
}
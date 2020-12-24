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

#include "compute_program.h"
#include "bgl.h"

#ifndef BEATMUP_OPENGLVERSION_GLES20

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

#endif

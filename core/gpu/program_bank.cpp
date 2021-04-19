/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include "program_bank.h"
#include "../gpu/recycle_bin.h"
#include <vector>

using namespace Beatmup;


GL::ProgramBank::~ProgramBank() {
    for (auto& it : this->programs)
        context.getGpuRecycleBin()->put(it.second.program);
}


GL::RenderingProgram* GL::ProgramBank::operator()(GraphicPipeline& gpu, const std::string& code, bool enableExternalTextures) {
    auto& cache = enableExternalTextures ? programsWithExtTex : programs;

    auto search = cache.find(code);
    if (search != cache.end()) {
        auto& holder = search->second;
        holder.userCount++;
        holder.program->enable(gpu);
        return holder.program;
    }

    // not found; create
    GL::Extensions ext = enableExternalTextures ? GL::Extensions::EXTERNAL_TEXTURE : GL::Extensions::NONE;
    GL::FragmentShader fragmentShader(gpu, code, Extensions::BEATMUP_DIALECT + ext);
    GL::RenderingProgram* program = new GL::RenderingProgram(gpu, fragmentShader);
    cache.emplace(std::make_pair(code, ProgramHolder{ program, 1 }));
    return program;
}


bool GL::ProgramBank::releaseProgram(GL::RenderingProgram* program, std::map<std::string, ProgramHolder>& cache) {
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        auto& holder = it->second;
        if (holder.program == program) {
            holder.userCount--;
            if (holder.userCount == 0) {
                delete program;
                cache.erase(it);
            }
            return true;
        }
    }
    return false;
}


void GL::ProgramBank::release(GraphicPipeline& gpu, GL::RenderingProgram* program) {
    if (!releaseProgram(program, programs) && !releaseProgram(program, programsWithExtTex))
        throw RuntimeError("No program found in a program bank");
}

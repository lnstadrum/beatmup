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


GL::RenderingProgram* GL::ProgramBank::operator()(GraphicPipeline& gpu, const std::string& code) {
    auto search = programs.find(code);
    if (search != programs.end()) {
        auto& holder = search->second;
        holder.userCount++;
        holder.program->enable(gpu);
        return holder.program;
    }

    // not found; create
    GL::FragmentShader fragmentShader(gpu, code);
    GL::RenderingProgram* program = new GL::RenderingProgram(gpu, fragmentShader);
    programs.emplace(std::make_pair(code, ProgramHolder{ program, 1 }));
    return program;
}


void GL::ProgramBank::release(GraphicPipeline& gpu, GL::RenderingProgram* program) {
    for (auto it = programs.begin(); it != programs.end(); ++it) {
        auto& holder = it->second;
        if (holder.program == program) {
            holder.userCount--;
            if (holder.userCount == 0) {
                delete program;
                programs.erase(it);
            }
            return;
        }
    }
    throw RuntimeError("No program found in a program bank");
}

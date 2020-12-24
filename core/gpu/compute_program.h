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
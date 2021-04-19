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

#pragma once
#include "../gpu/pipeline.h"
#include "../gpu/program.h"
#include "../context.h"
#include <string>
#include <map>

namespace Beatmup {
    namespace GL {
        /**
            Stores linked GLSL programs and their associated fragment shader codes.
        */
        class ProgramBank : public Object {
        private:
            typedef struct {
                GL::RenderingProgram* program;
                unsigned int userCount;
            } ProgramHolder;

            std::map<std::string, ProgramHolder> programs;              //!< map of source code to programs without external texture extension
            std::map<std::string, ProgramHolder> programsWithExtTex;    //!< map of source code to programs with external texture extension

            bool releaseProgram(GL::RenderingProgram* program, std::map<std::string, ProgramHolder>& cache);

        protected:
            Context& context;
        public:
            ProgramBank(Context& context) : context(context) {}
            ~ProgramBank();

            /**
                Provides a program given a fragment shader source code.
                Creates a new program or returns an available one increasing its user count (do not call this too often).
                \param[in] gpu                      A graphic pipeline instance
                \param[in] code                     The fragment shader code of the program
                \param[in] enableExternalTextures   If `true`, external texture extension is enabled in the program, for example, to access camera image in Android
                \return             Linked program.
            */
            GL::RenderingProgram* operator()(GraphicPipeline& gpu, const std::string& code, bool enableExternalTextures = false);

            /**
                Marks a program as unused any more. If the program has no other users, its is destroyed.
                \param[in] gpu          A graphic pipeline instance
                \param[in] program      The program
            */
            void release(GraphicPipeline& gpu, GL::RenderingProgram* program);
        };
    }
}

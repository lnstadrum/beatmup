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
#include "../utils/lockable_object.h"
#include "../color/matrix.h"
#include "program.h"
#include <map>
#include <string>

namespace Beatmup {
    namespace GL {

        /*
            Collection storing GLSL program parameters (scalars, matrices, vectors) to communicate them from user to GPU-managing thread.
        */
        class VariablesBundle : public LockableObject {
        private:

            class MatrixParameter {
            public:
                enum Type {
                    INT, FLOAT,
                    UNDEFINED
                };

            private:
                void* data;
                unsigned short int width, height;
                unsigned int count;
                Type type;

            public:
                MatrixParameter();
                ~MatrixParameter();
                MatrixParameter& operator= (MatrixParameter &&);

                void configure(Type type, unsigned short int width, unsigned short int height = 1, unsigned int count = 1);

                template<typename T> inline T* getData(int index = 0) const {
                    return (T*)data + width * height * index;
                }

                int getCount() const { return count; }
                int getWidth() const { return width; }
                int getHeight() const { return height; }
                Type getType() const { return type; }
            };

            std::map<std::string, int> integers;
            std::map<std::string, float> floats;
            std::map<std::string, MatrixParameter> params;

        protected:
            void apply(Program& program);

        public:

            void setInteger(std::string name, int value);
            void setInteger(std::string name, int x, int y);
            void setInteger(std::string name, int x, int y, int z);
            void setInteger(std::string name, int x, int y, int z, int w);

            /**
                Sets a scalar float uniform value.
                \param name     The uniform variable name
                \param value    The variable value
            */
            void setFloat(std::string name, float value);

            void setFloat(std::string name, float x, float y);
            void setFloat(std::string name, float x, float y, float z);
            void setFloat(std::string name, float x, float y, float z, float w);

            /**
                Sets a float 2*2 matrix variable value.
                \param name     The uniform variable name
                \param matrix   The variable value
            */
            void setFloatMatrix2(std::string name, const float matrix[4]);

            /**
                Sets a float 3*3 matrix variable value.
                \param name     The uniform variable name
                \param matrix   The variable value
            */
            void setFloatMatrix3(std::string name, const float matrix[9]);

            /**
                Sets a float 4*4 matrix variable value.
                \param name     The uniform variable name
                \param matrix   The variable value
            */
            void setFloatMatrix4(std::string name, const float matrix[16]);

            /**
                Sets a float 4*4 matrix variable value from a Color::Matrix instance.
                \param name     The uniform variable name
                \param matrix   The color matrix to copy values from
            */
            void setFloatMatrix4(std::string name, const Color::Matrix& matrix);

            /**
                Retrieves a value of a scalar float uniform variable by its name.
                \param name     The variable name
                \return the variable value, if defined previously, quiet NaN otherwise.
            */
            float getFloat(const std::string& name) const;
        };
    }
}
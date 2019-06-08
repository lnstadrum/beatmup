/*
	Collection storing GLSL program parameters (scalars, matrices, vectors)
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
			Collection storing GLSL program parameters (scalars, matrices, vectors) to communicate them from user to GPU-managing thread
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

			void setFloat(std::string name, float value);
			void setFloat(std::string name, float x, float y);
			void setFloat(std::string name, float x, float y, float z);
			void setFloat(std::string name, float x, float y, float z, float w);

			void setFloatMatrix3(std::string name, const float matrix[9]);

			void setFloatMatrix4(std::string name, const float matrix[16]);
			void setFloatMatrix4(std::string name, const Color::Matrix& matrix);

			float getFloat(const std::string& name) const;
		};
	}
}
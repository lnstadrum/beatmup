#include "variables_bundle.h"
#include "../bitmap/pixel_arithmetic.h"
#include "bgl.h"
using namespace Beatmup;
using namespace GL;
VariablesBundle::MatrixParameter& VariablesBundle::MatrixParameter::operator=(VariablesBundle::MatrixParameter&& src) {
    data = src.data;
    type = src.type;
    width = src.width;
    height = src.height;
    count = src.count;
    src.data = NULL;
    return *this;
}
VariablesBundle::MatrixParameter::MatrixParameter():
    data(nullptr), width(0), height(0), count(0), type(MatrixParameter::Type::UNDEFINED)
{}
void VariablesBundle::MatrixParameter::configure(Type type, unsigned short int width, unsigned short int height, unsigned int count) {
    free(data);
    this->width = width;
    this->height = height;
    this->count = count;
    this->type = type;
    switch (type) {
    case MatrixParameter::Type::INT:
        data = malloc(sizeof(GLint) * width*height*count);
        break;
    case MatrixParameter::Type::FLOAT:
        data = malloc(sizeof(GLfloat) * width*height*count);
        break;
    default:
        Insanity::insanity("Invalid data type when construction matrix parameter");
    }
}
VariablesBundle::MatrixParameter::~MatrixParameter() {
    free(data);
}
void VariablesBundle::setInteger(std::string name, int value) {
    integers[name] = value;
}
void VariablesBundle::setInteger(std::string name, int x, int y) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::INT, 2);
    GLint* data = param.getData<GLint>();
    data[0] = x;
    data[1] = y;
}
void VariablesBundle::setInteger(std::string name, int x, int y, int z) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::INT, 3);
    GLint* data = param.getData<GLint>();
    data[0] = x;
    data[1] = y;
    data[2] = z;
}
void VariablesBundle::setInteger(std::string name, int x, int y, int z, int w) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::INT, 4);
    GLint* data = param.getData<GLint>();
    data[0] = x;
    data[1] = y;
    data[2] = z;
    data[3] = w;
}
void VariablesBundle::setFloat(std::string name, float value) {
    floats[name] = value;
}
void VariablesBundle::setFloat(std::string name, float x, float y) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::FLOAT, 2);
    GLfloat* data = param.getData<GLfloat>();
    data[0] = x;
    data[1] = y;
}
void VariablesBundle::setFloat(std::string name, float x, float y, float z) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::FLOAT, 3);
    GLfloat* data = param.getData<GLfloat>();
    data[0] = x;
    data[1] = y;
    data[2] = z;
}
void VariablesBundle::setFloat(std::string name, float x, float y, float z, float w) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::FLOAT, 4);
    GLfloat* data = param.getData<GLfloat>();
    data[0] = x;
    data[1] = y;
    data[2] = z;
    data[3] = w;
}
void VariablesBundle::setFloatMatrix3(std::string name, const float matrix[9]) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::FLOAT, 3, 3);
    GLfloat *out = param.getData<GLfloat>();
    for (const float* in = matrix; in < matrix + 9; ++in)
        *out++ = *in;
}
void VariablesBundle::setFloatMatrix4(std::string name, const float matrix[16]) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::FLOAT, 4, 4);
    GLfloat *out = param.getData<GLfloat>();
    for (const float* in = matrix; in < matrix + 16; ++in)
        *out++ = *in;
}
void VariablesBundle::setFloatMatrix4(std::string name, const Color::Matrix& matrix) {
    MatrixParameter& param = params[name];
    param.configure(MatrixParameter::Type::FLOAT, 4, 4);
    GLfloat *out = param.getData<GLfloat>();
    for (int x = 0; x < 4; ++x) {
        const pixfloat4 row = matrix[x];
        for (int y = 0; y < 4; ++y)
            *out++ = row[y];
    }
}
float VariablesBundle::getFloat(const std::string& name) const {
    const auto& it = floats.find(name);
    if (it == floats.cend())
        return std::numeric_limits<float>::quiet_NaN();
    return it->second;
}
void VariablesBundle::apply(Program& program) {
    for (auto& var : integers)
        program.setInteger(var.first.c_str(), var.second);
    for (auto& var : floats)
        program.setFloat(var.first.c_str(), var.second);
    for (auto& var : params) {
        // assign a vector
        if (var.second.getHeight() == 1 && var.second.getCount() == 1)
            switch (var.second.getType()) {
                case MatrixParameter::Type::INT: {
                    GLint* data = var.second.getData<GLint>();
                    switch (var.second.getWidth()) {
                        case 2:
                            glUniform2i(program.getUniformLocation(var.first.c_str()), data[0], data[1]);
                            break;
                        case 3:
                            glUniform3i(program.getUniformLocation(var.first.c_str()), data[0], data[1], data[2]);
                            break;
                        case 4:
                            glUniform4i(program.getUniformLocation(var.first.c_str()), data[0], data[1], data[2], data[3]);
                            break;
                        default: Insanity::insanity("Invalid parameter size");
                    }
                    break;
                }
                case MatrixParameter::Type::FLOAT: {
                    GLfloat* data = var.second.getData<GLfloat>();
                    switch (var.second.getWidth()) {
                        case 2:
                            glUniform2f(program.getUniformLocation(var.first.c_str()), data[0], data[1]);
                            break;
                        case 3:
                            glUniform3f(program.getUniformLocation(var.first.c_str()), data[0], data[1], data[2]);
                            break;
                        case 4:
                            glUniform4f(program.getUniformLocation(var.first.c_str()), data[0], data[1], data[2], data[3]);
                            break;
                        default: Insanity::insanity("Invalid parameter size");
                    }
                    break;
                }
                default: Insanity::insanity("Invalid parameter type");
            }
        // assign an array
        else if (var.second.getHeight() == 1)
#define CASE(n,t,T) case n: glUniform##n##t##v(program.getUniformLocation(var.first.c_str()), var.second.getCount(), var.second.getData<T>()); break;
            switch (var.second.getType()) {
                case MatrixParameter::Type::INT:
                    switch (var.second.getWidth()) {
                        CASE(1, i, GLint);
                        CASE(2, i, GLint);
                        CASE(3, i, GLint);
                        CASE(4, i, GLint);
                        default: Insanity::insanity("Invalid parameter size");
                    }
                    break;
                case MatrixParameter::Type::FLOAT:
                    switch (var.second.getWidth()) {
                        CASE(1, f, GLfloat);
                        CASE(2, f, GLfloat);
                        CASE(3, f, GLfloat);
                        CASE(4, f, GLfloat);
                        default: Insanity::insanity("Invalid parameter size");
                    }
                    break;
                default: Insanity::insanity("Invalid parameter type");
            }
        // assign a matrix
        else if (var.second.getWidth() == var.second.getHeight())
#undef CASE
#define CASE(n) case n: glUniformMatrix##n##fv(program.getUniformLocation(var.first.c_str()), var.second.getCount(), GL_FALSE, var.second.getData<GLfloat>()); break;
            switch (var.second.getType()) {
                case MatrixParameter::Type::FLOAT: {
                    switch (var.second.getWidth()) {
                        CASE(2);
                        CASE(3);
                        CASE(4);
                        default: Insanity::insanity("Invalid parameter size");
                    }
                    break;
                }
                default: Insanity::insanity("Invalid parameter type");
            }
        // an error otherwise
        else
            Insanity::insanity("Invalid matrix size");
        GL::GLException::check((std::string("setting uniform variable ") + var.first).c_str());
    }}
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

#include "linear_mapping.h"
#include "../utils/fixed_point.h"
#include "../utils/string_builder.h"
#include "../gpu/bgl.h"
#include <cstring>

using namespace Beatmup;
using namespace GL;

#ifdef BEATMUP_OPENGLVERSION_GLES20
const Vector::Format Vector::DEFAULT_FORMAT = Vector::Format::FIXED16;
#else
const Vector::Format Vector::DEFAULT_FORMAT = Vector::Format::FLOAT;
#endif

static const char* UNIFORM_MATRIX = "mtrx";
static const char* UNIFORM_INPUT = "inp";
static const char* UNIFORM_BIAS = "bias";
static const char* UNIFORM_DELTA = "dt";

static const int
    VECTOR_TEXTURE_DIMS = 2,    // actual number of dimensions in textures storing input and bias vectors
    VECTOR_MAIN_DIM = 1;        // main (non-singleton) dimension in textures containing input and bias


static inline bool useFixedPointStorage(bool forceFixed16Storage) {
#ifdef BEATMUP_OPENGLVERSION_GLES20
    return true;
#else
    return forceFixed16Storage;
#endif
}


/**
    \internal
    Adds GLSL statement sampling a vector component.
    \param declaration      If declaring a new variable, its datatype (empty string otherwise)
    \param variable         GLSL variable to assign the sampled value to
    \param uniform          The uniform variable to sample
    \param coordinate       Texture coordinate
    \param delta            If non-negative a delta with the corresponding index is added to the texture coordinate
*/
void sampleVectorComponent(String& code, const char* declaration, const char* variable, const char* uniform, const char* coordinate, int delta = -1) {
    code.printf("%s %s = texture%dD(%s, vec%d(", declaration, variable, VECTOR_TEXTURE_DIMS, uniform, VECTOR_TEXTURE_DIMS);
    for (int i = 0; i < VECTOR_TEXTURE_DIMS; ++i) {
        if (i > 0)
            code(",");
        if (i == VECTOR_MAIN_DIM) {
            if (delta >= 0) {
                code.printf("%s + %s[%d]", coordinate, UNIFORM_DELTA, delta);
            }
            else {
                code(coordinate);
            }
        }
        else
            code("0");
    }
    code.line("));");
}


/**
    \brief Generates GLSL code of float to 16-bit fixed point packing/unpacking functions.
    \param[in,out] code             String builder containing the shader source code
    \param[in] suffix               A suffix to add to the function names
    \param[in] pack                 If `true`, the packing function code is added
    \param[in] unpackVec            If `true`, the vector unpacking function code is added (producing a vec4 from two vec4)
    \param[in] unpackScalar         If `true`, the scalar unpacking function code is added (producing a float from two floats)
    \param[in] packPrecision        Number of bits storing the fractional part in the resulting packed value
    \param[in] unpackPrecision      Number of bits storing the fractional part in the input value to unpack
*/
static inline void declareGlsl16bitFixedPointFunctions(StringBuilder& code, const char* suffix, bool pack, bool unpackVec, bool unpackScalar, unsigned int packPrecision = 8, unsigned int unpackPrecision = 8) {
    if (pack) {
        code.printf("lowp vec2 pack%s(highp float v) {", suffix);
        if (packPrecision != 8)
            code.printf("v *= %0.1f;", packPrecision > 8 ? (float)(1 << (packPrecision - 8)) : 1.0f / (1 << (packPrecision - 8)));
        code.line(
            "  return vec2((256.0 / 255.0) * fract(v), (1.0 / 255.0) * floor(v) + (128.0 / 255.0));"
            "}"
        );
    }
    const float
        scaleMsb = unpackPrecision > 8 ? 1.0f / (1 << (unpackPrecision - 8)) : (float)(1 << (unpackPrecision - 8)),
        scaleLsb = 255.0f * scaleMsb;
    if (unpackVec)
        code.printf(
            "highp vec4 unpack%s(lowp vec4 lsb, lowp vec4 msb) {"
            "  return lsb * (%0.5f / 256.0) + floor(255.0 * msb - 127.5) * %0.5f;"
            "}",
            suffix, scaleLsb, scaleMsb
        );
    if (unpackScalar)
        code.printf(
            "highp float unpack%s(lowp float lsb, lowp float msb) {"
            "  return lsb * (%0.5f / 256.0) + floor(255.0 * msb - 127.5) * %0.5f;"
            "}",
            suffix, scaleLsb, scaleMsb
        );
}


/**
    Converts 16-bit fixed-point number into a floating point number.
    \param[in] lsb      The least significant byte of the input fixed-point value
    \param[in] msb      The most significant byte of the input fixed-point value
    \return the floating-point value.
*/
static inline float unpackFloatFrom16bit(const uint8_t lsb, const int msb) {
    return (float)Fixed16<8>::interpret( ((msb - 128) << 8) + lsb );
}


/**
    \brief Packs a floating point value into a 16-bit fixed-point value.
    The resulting representation matches the formulas in unpackFloatFrom16bit(..) and packFloatTo16bit(..)
    A biased clamping-friendly formulation is used: "0.0" on input produces the most significant channel value of "0.5".
    \param[in] value    The input value to pack
    \param[in] lsb      The least significant byte in the output fixed-point value
    \param[in] msb      The most significant byte in the output fixed-point value
*/
static inline void packFloatTo16bit(const float value, uint8_t& lsb, uint8_t& msb) {
    union {
        Fixed16<8> val;
        uint16_t bytes;
    } ptr{ value };
    lsb = ptr.bytes & 0xff;
    msb = (ptr.bytes >> 8) + 128;
}


static inline void findMinMax(const float* values, const int count, float& minVal, float& maxVal) {
    minVal = maxVal = values[0];
    for (int i = 1; i < count; ++i) {
        minVal = std::min(minVal, values[i]);
        maxVal = std::max(maxVal, values[i]);
    }
}


/**
    Real-valued matrix usable by GPU
*/
class LinearMapping::Matrix : public TextureHandler {
    friend class LinearMapping;
private:
    const TextureFormat format;
    const int texWidth, texHeight;  //!< texture size
    const int width, height;        //!< matrix size
    float mapScale;                 //!< scaling applied to the matrix coefficients to optimize the fixed-point range use
    float mapOffset;                //!< offset applied to the matrix coefficients after scaling to optimize the fixed-point range use

    void prepare(GraphicPipeline& gpu, bool);

public:
    Matrix(GraphicPipeline& gpu, int width, int height, const bool floatingPoint);

    /**
        \brief Creates a matrix in GPU memory.
        The memory layout depends on the data format. For floating point data four consecutive rows are packed into color channels.
        The first texture row looks like this ("x,y" for "col,row" in the input matrix):
            r   0,0    1,0    2,0    3,0    4,0 ...
            g   0,1    1,1    2,1    3,1    4,1 ...
            b   0,2    1,2    2,2    3,2    4,2 ...
            a   0,3    1,3    2,3    3,3    4,3 ...
        In 16-bit fixed point case, two consecutive columns are packed into color channels, alternating the least and most significant bytes:
            r   0,0L   0,0M   0,1L   0,1M ...
            g   1,0L   1,0M   1,1L   1,1M ...
            b   2,0L   2,0M   2,1L   2,1M ...
            a   3,0L   3,0M   3,1L   3,1M ...
        \param[in] gpu              A graphic pipeline instance
        \param[in] width            Matrix width (number of columns)
        \param[in] height           Matrix height (number of rows)
        \param[in] values           Matrix coefficients in scanline order (rows)
        \param[in] floatingPoint    If `true`, the matrix coefficients are stored in floating point format (16-bit fixed point otherwise).
    */
    Matrix(GraphicPipeline& gpu, int width, int height, const float* values, const bool floatingPoint);

    void bind(GraphicPipeline& gpu, int textureUnit) const;

    inline int getMatrixWidth() const { return width; }
    inline int getMatrixHeight() const { return height; }
    inline float getScale() const { return mapScale; }
    inline float getOffset() const { return mapOffset; }

    inline const int getWidth() const { return texWidth; }
    inline const int getHeight() const { return texHeight; }
    inline const int getDepth() const { return 1; }
    inline const TextureFormat getTextureFormat() const { return format; }
};


LinearMapping::Matrix::Matrix(GraphicPipeline& gpu, int width, int height, const bool floatingPoint) :
    format(floatingPoint ? TextureFormat::RGBAx32f : TextureFormat::RGBAx8),
    texWidth(width), texHeight(floatingPoint ? height / 4 : height / 2),
    width(width), height(height), mapScale(1), mapOffset(0)
{
#ifdef BEATMUP_OPENGLVERSION_GLES20
    if (floatingPoint)
        throw RuntimeError("Floating-point linear mapping is not supported in ES 2.0");
#endif
    RuntimeError::check(height % 4 == 0, "Matrix height must be a multiple of four.");

    // init texture with zeros
    const int numPix = 4 * getWidth() * getHeight();
    std::vector<uint8_t> zeros(numPix * TextureHandler::TEXTURE_FORMAT_BYTES_PER_PIXEL[format]);
    if (floatingPoint)
        memset(zeros.data(), 0, zeros.size());
    else
        for (int i = 0; i < numPix; i += 2)
            packFloatTo16bit(0, zeros[i], zeros[i + 1]);

    // setup texture
    glGenTextures(1, &textureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
#ifdef BEATMUP_OPENGLVERSION_GLES20
    glTexImage2D(GL_TEXTURE_2D,
        0,
        BITMAP_INTERNALFORMATS[format],
        getWidth(), getHeight(),
        0,
        BITMAP_PIXELFORMATS[format],
        BITMAP_PIXELTYPES[format],
        zeros.data()
    );
#else
    glTexStorage2D(GL_TEXTURE_2D, 1, BITMAP_INTERNALFORMATS[format], getWidth(), getHeight());
    glTexSubImage2D(GL_TEXTURE_2D,
        0, 0, 0, getWidth(), getHeight(),
        BITMAP_PIXELFORMATS[format],
        BITMAP_PIXELTYPES[format],
        zeros.data()
    );
#endif
}


LinearMapping::Matrix::Matrix(GraphicPipeline& gpu, int width, int height, const float* values, const bool floatingPoint):
    format(floatingPoint ? TextureFormat::RGBAx32f : TextureFormat::RGBAx8),
    texWidth(width),
    texHeight(floatingPoint ? height / 4 : height / 2),
    width(width), height(height)
{
#ifdef BEATMUP_OPENGLVERSION_GLES20
    if (floatingPoint)
        throw RuntimeError("Floating-point linear mapping is not supported in ES 2.0");
#endif
    if (floatingPoint)
        RuntimeError::check(height % 4 == 0, "Matrix height must be a multiple of four.");
    else {
        RuntimeError::check(width % 4 == 0, "Matrix width must be a multiple of four.");
        RuntimeError::check(height % 2 == 0, "Matrix height must be pair.");
    }

    // init texture handler
    glGenTextures(1, &textureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    // floating point compute mode: r, g, b, a store consecutive rows, not columns
    if (floatingPoint) {
#ifndef BEATMUP_OPENGLVERSION_GLES20
        std::vector<float> textureData(4 * getWidth() * getHeight());
        for (int y = 0, i = 0; y < height; y += 4)
            for (int x = 0; x < width; ++x, i += 4) {
                textureData[i + 0] = values[y * width + x];
                textureData[i + 1] = values[(y + 1) * width + x];
                textureData[i + 2] = values[(y + 2) * width + x];
                textureData[i + 3] = values[(y + 3) * width + x];
            }

        glTexStorage2D(GL_TEXTURE_2D, 1, BITMAP_INTERNALFORMATS[format], getWidth(), getHeight());
        glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0, getWidth(), getHeight(),
            BITMAP_PIXELFORMATS[format],
            BITMAP_PIXELTYPES[format],
            textureData.data()
        );
#endif
    }

    // fixed point compute mode: r, g, b, a store 4 consecutive rows (all LSB or all MSB)
    else {
        // measure magnitude
        float minVal, maxVal;
        findMinMax(values, width * height, minVal, maxVal);
        mapScale = maxVal > minVal ? (Fixed16<8>::max() - Fixed16<8>::min()) / (maxVal - minVal) : 1;
        mapOffset = maxVal > minVal ? Fixed16<8>::min() - minVal * mapScale : 0;

        // convert to fixed point
        std::vector<uint8_t> textureData(4 * getWidth() * getHeight());
        for (int y = 0, i = 0; y < height; y += 2)
            for (int x = 0; x < width; x += 4, i += 16) {
                packFloatTo16bit(mapOffset + mapScale * values[y * width + x + 0], textureData[i + 0], textureData[i + 4]);
                packFloatTo16bit(mapOffset + mapScale * values[y * width + x + 1], textureData[i + 1], textureData[i + 5]);
                packFloatTo16bit(mapOffset + mapScale * values[y * width + x + 2], textureData[i + 2], textureData[i + 6]);
                packFloatTo16bit(mapOffset + mapScale * values[y * width + x + 3], textureData[i + 3], textureData[i + 7]);
                packFloatTo16bit(mapOffset + mapScale * values[(y + 1) * width + x + 0], textureData[i + 8], textureData[i + 12]);
                packFloatTo16bit(mapOffset + mapScale * values[(y + 1) * width + x + 1], textureData[i + 9], textureData[i + 13]);
                packFloatTo16bit(mapOffset + mapScale * values[(y + 1) * width + x + 2], textureData[i + 10], textureData[i + 14]);
                packFloatTo16bit(mapOffset + mapScale * values[(y + 1) * width + x + 3], textureData[i + 11], textureData[i + 15]);
            }

#ifdef BEATMUP_OPENGLVERSION_GLES20
        glTexImage2D(GL_TEXTURE_2D,
            0,
            BITMAP_INTERNALFORMATS[format],
            getWidth(), getHeight(),
            0,
            BITMAP_PIXELFORMATS[format],
            BITMAP_PIXELTYPES[format],
            textureData.data()
        );
#else
        glTexStorage2D(GL_TEXTURE_2D, 1, BITMAP_INTERNALFORMATS[format], getWidth(), getHeight());
        glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0, getWidth(), getHeight(),
            BITMAP_PIXELFORMATS[format],
            BITMAP_PIXELTYPES[format],
            textureData.data()
        );
#endif
    }
}


void LinearMapping::Matrix::prepare(GraphicPipeline& gpu, bool) {
    glBindTexture(GL_TEXTURE_2D, textureHandle);
}


void LinearMapping::Matrix::bind(GraphicPipeline& gpu, int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef BEATMUP_DEBUG
    GLException::check("binding matrix");
#endif
}


Vector::Vector(Context& context, GraphicPipeline& gpu, const int size, const Format format) :
    context(context),
    texFormat(format == Format::FLOAT ? TextureHandler::TextureFormat::RGBAx32f : TextureHandler::TextureFormat::RGBAx8),
    format(format),
    size(size),
    mapScale(1), mapOffset(0)
{
#ifdef BEATMUP_OPENGLVERSION_GLES20
    if (format == Format::FLOAT)
        throw RuntimeError("Floating-point vectors are not supported in ES 2.0");
#endif
    RuntimeError::check(size % 4 == 0, "Vector size must be a multiple of four.");

    // init texture handler
    glGenTextures(1, &textureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

#ifdef BEATMUP_OPENGLVERSION_GLES20
    glTexImage2D(GL_TEXTURE_2D,
        0,
        BITMAP_INTERNALFORMATS[texFormat],
        getWidth(), getHeight(),
        0,
        BITMAP_PIXELFORMATS[texFormat],
        BITMAP_PIXELTYPES[texFormat],
        nullptr
    );
#else
    glTexStorage2D(GL_TEXTURE_2D, 1, BITMAP_INTERNALFORMATS[texFormat], getWidth(), getHeight());
#endif
}


Vector::Vector(Context& context, GraphicPipeline& gpu, const int size, const Format format, const float* values, bool remap) :
    context(context),
    texFormat(format == Format::FLOAT ? TextureHandler::TextureFormat::RGBAx32f : TextureHandler::TextureFormat::RGBAx8),
    format(format),
    size(size),
    mapScale(1), mapOffset(0)
{
#ifdef BEATMUP_OPENGLVERSION_GLES20
    if (format == Format::FLOAT)
        throw RuntimeError("Floating-point vectors are not supported in ES 2.0");
#endif
    RuntimeError::check(size % 4 == 0, "Vector size must be a multiple of four.");

    glGenTextures(1, &textureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    if (format != Format::FLOAT) {
        // remap
        if (remap) {
            float minVal, maxVal;
            findMinMax(values, size, minVal, maxVal);
            if (maxVal > minVal) {
                if (format == Format::FIXED16) {
                    mapScale = (Fixed16<8>::max() - Fixed16<8>::min()) / (maxVal - minVal);
                    mapOffset = Fixed16<8>::min() - minVal * mapScale;
                }
                else if (format == Format::TEXTURE) {
                    mapScale = 1.0f / (maxVal - minVal);
                    mapOffset = - minVal * mapScale;
                }
            }
        }

        // convert data from floating point
        std::vector<uint8_t> textureData(4 * getHeight());
        if (format == Format::FIXED16)
            for (int i = 0; i < size; ++i)
                packFloatTo16bit(mapOffset + mapScale * values[i], textureData[2 * i], textureData[2 * i + 1]);
        else if (format == Format::TEXTURE)
            for (int i = 0; i < size; ++i) {
                const float val = mapOffset + mapScale * values[i];
                textureData[i] = val <= 0.0f ? 0 : val >= 1.0f ? 255 : (uint8_t)roundf_fast(val * 255);
            }
        else
            Insanity::insanity("Invalid vector data format");

#ifdef BEATMUP_OPENGLVERSION_GLES20
        glTexImage2D(GL_TEXTURE_2D,
            0,
            BITMAP_INTERNALFORMATS[texFormat],
            getWidth(), getHeight(),
            0,
            BITMAP_PIXELFORMATS[texFormat],
            BITMAP_PIXELTYPES[texFormat],
            textureData.data()
        );
#else
        glTexStorage2D(GL_TEXTURE_2D, 1, BITMAP_INTERNALFORMATS[texFormat], getWidth(), getHeight());
        glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0, getWidth(), getHeight(),
            BITMAP_PIXELFORMATS[texFormat],
            BITMAP_PIXELTYPES[texFormat],
            textureData.data()
        );
#endif
    }

    else {
#ifndef BEATMUP_OPENGLVERSION_GLES20
        glTexStorage2D(GL_TEXTURE_2D, 1, BITMAP_INTERNALFORMATS[texFormat], getWidth(), getHeight());
        glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0, getWidth(), getHeight(),
            BITMAP_PIXELFORMATS[texFormat],
            BITMAP_PIXELTYPES[texFormat],
            values
        );
#endif
    }
}


Vector::~Vector() {
    invalidate(*context.getGpuRecycleBin());
}


void Vector::prepare(GraphicPipeline& gpu) {
    glBindTexture(GL_TEXTURE_2D, textureHandle);
}


void Vector::fetch(GraphicPipeline& gpu, std::vector<float>& output) const {
    output.resize(size);
    gpu.bindOutput(textureHandle);

    if (format == Format::FLOAT)
        glReadPixels(0, 0, getWidth(), getHeight(),
            GL_RGBA, GL_FLOAT,
            output.data()
        );
    else if (format == Format::FIXED16) {
        glReadPixels(0, 0, getWidth(), getHeight(),
            GL_RGBA, GL_UNSIGNED_BYTE,
            (void*)output.data()
        );

        // convert values to float; use the same buffer, scan in inverse order
        const uint8_t* ptr = (const uint8_t*)output.data() + 2 * size;
        const size_t lastNum = output.size() - 1;
        for (size_t i = 0; i <= lastNum; ++i) {
            ptr -= 2;
            output[lastNum - i] = unpackFloatFrom16bit(ptr[0], ptr[1]);
        }
    }
    else if (format == Format::TEXTURE) {
        glReadPixels(0, 0, getWidth(), getHeight(),
            GL_RGBA, GL_UNSIGNED_BYTE,
            (void*)output.data()
        );

        // convert values to float; use the same buffer, scan in inverse order
        const uint8_t* ptr = (const uint8_t*)output.data() + size;
        const size_t lastNum = output.size() - 1;
        for (size_t i = 0; i <= lastNum; ++i) {
            --ptr;
            output[lastNum - i] = *ptr / 255.0f;
        }
    }
    else
        Insanity::insanity("Unsupported vector format");

    // remap back if needed
    if (mapScale != 1 || mapOffset != 0)
        for (auto& _ : output)
            _ = (_ - mapOffset) / mapScale;
}


size_t Vector::getMemorySize() const {
    switch (format) {
        case Format::FLOAT:
            return sizeof(float) * (size_t)size;
        case Format::FIXED16:
            return 2 * (size_t)size;
        case Format::TEXTURE:
            return (size_t)size;
    }
    Insanity::insanity("Unsupported vector format");
    return 0;
}


LinearMapping::LinearMapping(Context& context, bool forceFixed16Storage) :
    context(context),
    buffer{ nullptr, nullptr }, matrix(nullptr), bias(nullptr),
    multStage(nullptr), sumStage(nullptr), lastSumStage(nullptr), programBank(nullptr),
    leftPadding(SUM_STAGE_STEPS), forceFixed16Storage(forceFixed16Storage), fixed16Input(false), fixed16Output(false), ready(false)
{}


LinearMapping::~LinearMapping() {
    if (matrix) {
        matrix->invalidate(*context.getGpuRecycleBin());
        delete matrix;
    }
    if (buffer[0]) {
        buffer[0]->invalidate(*context.getGpuRecycleBin());
        delete buffer[0];
    }
    if (buffer[1]) {
        buffer[1]->invalidate(*context.getGpuRecycleBin());
        delete buffer[1];
    }
    if (!programBank)
        context.getGpuRecycleBin()->put({ multStage, sumStage, lastSumStage });
}


void LinearMapping::setMatrix(GraphicPipeline& gpu, const int width, const int height, const float* values) {
    if (bias)
        RuntimeError::check(bias->getSize() == height, "Matrix height does not match bias vector length");
    RuntimeError::check(width % (4 * MULT_STAGE_STEPS) == 0, "Matrix width is not a multiple of " + std::to_string(4 * MULT_STAGE_STEPS));
    if (matrix) {
        matrix->invalidate(*context.getGpuRecycleBin());
        delete matrix;
    }
    matrix = new Matrix(gpu, width, height, values, !useFixedPointStorage(forceFixed16Storage));
    ready = false;
}


void LinearMapping::setBias(GraphicPipeline& gpu, const int height, const float* values) {
    if (matrix)
        RuntimeError::check(height == matrix->getMatrixHeight(), "Matrix height does not match bias vector length.");
    if (bias)
        delete bias;
    bias = new Vector(context, gpu, height, useFixedPointStorage(forceFixed16Storage) ? Vector::Format::FIXED16 : Vector::Format::FLOAT, values, true);
    ready = false;
}


void LinearMapping::prepare(GraphicPipeline& gpu, TextureHandler& output, TextureHandler& input, ProgramBank* bank) {
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(input.getNumberOfChannels() == 4, "4-channel input texture handler expected");
    DebugAssertion::check(output.getNumberOfChannels() == 4, "4-channel output texture handler expected");
#endif

    const bool isPlainInput = (4 * input.getHeight() == matrix->getMatrixWidth());        // input samples are just r, g, b, a values in the texture
    const bool isPackedInput = (2 * input.getHeight() == matrix->getMatrixWidth());       // input samples are packed in (r, g) and (b, a) pairs
    const bool isPlainOutput = (4 * output.getHeight() == matrix->getMatrixHeight());
    const bool isPackedOutput = (2 * output.getHeight() == matrix->getMatrixHeight());

    InvalidArgument::check(isPlainInput || isPackedInput, "Input vector height does not match matrix width");
    InvalidArgument::check(isPlainOutput || isPackedOutput, "Output vector height does not match matrix height");

    if (ready && fixed16Input == isPackedInput && fixed16Output == isPackedOutput)
        // nothing to do, ready to go
        return;

    fixed16Input = isPackedInput;
    fixed16Output = isPackedOutput;

    if (!matrix)
        throw RuntimeError("No matrix");

    // removing old programs
    if (programBank) {
        if (multStage) programBank->release(gpu, multStage);
        if (sumStage) programBank->release(gpu, sumStage);
        if (lastSumStage) programBank->release(gpu, lastSumStage);
    }
    else {
        delete multStage;
        delete sumStage;
        delete lastSumStage;
    }

    // reseting state
    programBank = bank;

    const unsigned int precisionBoost = 2;      // additional precision bits on intermediate stages
    const bool fixedPointStorage = useFixedPointStorage(forceFixed16Storage);

    // make multiplication stage program: this one multiplies small pieces of the matrix by small pieces of the input vector storing the results in a texture
    {
        String code(BEATMUP_SHADER_CODE_V(
            varying highp vec2 texCoord;
        ));

        code.printf("uniform sampler%dD %s;", VECTOR_TEXTURE_DIMS, UNIFORM_INPUT);
        code.printf("uniform sampler2D %s;", UNIFORM_MATRIX);
        code.printf("uniform highp float %s[%d];", UNIFORM_DELTA, multStageDelta.size());
        declareGlsl16bitFixedPointFunctions(code, "", fixedPointStorage, fixedPointStorage, false, 8 + precisionBoost);
        declareGlsl16bitFixedPointFunctions(code, "In", false, fixed16Input, false);
        code.nl();
        code.line("void main() {");

        // first block: sample the matrix
        if (!fixedPointStorage) {
            // read floating-point matrix
            code.printf("highp mat4 m = mat4("
                "texture2D(%s, texCoord),"
                "texture2D(%s, vec2(texCoord.x+%s[0], texCoord.y)),"
                "texture2D(%s, vec2(texCoord.x+%s[1], texCoord.y)),"
                "texture2D(%s, vec2(texCoord.x+%s[2], texCoord.y)));",
                UNIFORM_MATRIX,
                UNIFORM_MATRIX, UNIFORM_DELTA,
                UNIFORM_MATRIX, UNIFORM_DELTA,
                UNIFORM_MATRIX, UNIFORM_DELTA
            );
        }
        else {
            // read fixed-point packed matrix
            code.printf("highp vec4 m1 = unpack(texture2D(%s, texCoord), texture2D(%s, vec2(texCoord.x+%s[0], texCoord.y)));",
                UNIFORM_MATRIX, UNIFORM_MATRIX, UNIFORM_DELTA);
            code.printf("highp vec4 m2 = unpack(texture2D(%s, vec2(texCoord.x+%s[1], texCoord.y)), texture2D(%s, vec2(texCoord.x+%s[2], texCoord.y)));",
                UNIFORM_MATRIX, UNIFORM_DELTA, UNIFORM_MATRIX, UNIFORM_DELTA);
            code.printf("m1 = (m1 - %0.8f) * %0.8f;", matrix->getOffset(), 1 / matrix->getScale());
            code.printf("m2 = (m2 - %0.8f) * %0.8f;", matrix->getOffset(), 1 / matrix->getScale());
        }

        // sample the vector
        if (fixed16Input) {
            sampleVectorComponent(code, "lowp vec4", "vp1", UNIFORM_INPUT, "texCoord.x");
            sampleVectorComponent(code, "lowp vec4", "vp2", UNIFORM_INPUT, "texCoord.x", 1);
            // need to de-multiplex components: LMLM+LMLM to LLLL+MMMM
            code("highp vec4 v = unpackIn(vec4(vp1.xz, vp2.xz), vec4(vp1.yw, vp2.yw));");
        }
        else {
            sampleVectorComponent(code, "highp vec4", "v", UNIFORM_INPUT, "texCoord.x");
        }

        // compute the result
        if (fixedPointStorage)
            code.line("highp vec2 r = vec2(dot(m1, v), dot(m2, v));");
        else
            code.line("highp vec4 r = m * v;");

        // further blocks
        for (int blockIdx = 1; blockIdx < MULT_STAGE_STEPS; ++blockIdx) {
            // advance the texture coord
            if (blockIdx == 1)
                code.printf("highp float x = texCoord.x + %s[3];", UNIFORM_DELTA);
            else {
                code.printf("x += %s[3];", UNIFORM_DELTA);
            }

            // sample the matrix
            if (!fixedPointStorage)
                // read floating-point matrix
                code.printf("m = mat4("
                    "texture2D(%s, vec2(x, texCoord.y)),"
                    "texture2D(%s, vec2(x+%s[0], texCoord.y)),"
                    "texture2D(%s, vec2(x+%s[1], texCoord.y)),"
                    "texture2D(%s, vec2(x+%s[2], texCoord.y)));",
                    UNIFORM_MATRIX,
                    UNIFORM_MATRIX, UNIFORM_DELTA,
                    UNIFORM_MATRIX, UNIFORM_DELTA,
                    UNIFORM_MATRIX, UNIFORM_DELTA
                );
            else {
                // read fixed-point packed matrix
                code.printf("m1 = unpack(texture2D(%s, vec2(x, texCoord.y)), texture2D(%s, vec2(x+%s[0], texCoord.y)));",
                    UNIFORM_MATRIX, UNIFORM_MATRIX, UNIFORM_DELTA);
                code.printf("m2 = unpack(texture2D(%s, vec2(x+%s[1], texCoord.y)), texture2D(%s, vec2(x+%s[2], texCoord.y)));",
                    UNIFORM_MATRIX, UNIFORM_DELTA, UNIFORM_MATRIX, UNIFORM_DELTA);
                code.printf("m1 = (m1 - %0.8f) * %0.8f;", matrix->getOffset(), 1 / matrix->getScale());
                code.printf("m2 = (m2 - %0.8f) * %0.8f;", matrix->getOffset(), 1 / matrix->getScale());
            }

            // sample the vector
            if (fixed16Input) {
                sampleVectorComponent(code, "", "vp1", UNIFORM_INPUT, "x");
                sampleVectorComponent(code, "", "vp2", UNIFORM_INPUT, "x", 1);
                // need to demultiplex components: LMLM+LMLM to LLLL+MMMM
                code("v = unpackIn(vec4(vp1.xz, vp2.xz), vec4(vp1.yw, vp2.yw));");
            }
            else {
                sampleVectorComponent(code, "", "v", UNIFORM_INPUT, "x");
            }

            // compute the result
            if (fixedPointStorage)
                code.line("r += vec2(dot(m1, v), dot(m2, v));");
            else
                code.line("r += m * v;");
        }

        // store the result
        if (fixedPointStorage)
            code("gl_FragColor = vec4(pack(r.x), pack(r.y));");
        else
            code.line("gl_FragColor = r;");

        code("}");

        if (bank)
            multStage = (*bank)(gpu, code);
        else
            multStage = new RenderingProgram(gpu, FragmentShader(gpu, code));
    }

    // set up an intermediate buffer
    delete buffer[0];
    buffer[0] = new Matrix(gpu, matrix->getMatrixWidth() / (4 * MULT_STAGE_STEPS) + leftPadding, matrix->getMatrixHeight(), !fixedPointStorage);

    // compute multiplication stage delta and texture coordinates
    for (size_t i = 0; i < multStageDelta.size(); ++i)
        multStageDelta[i] = (float)(i + 1) / matrix->getWidth();

    multStageTexCoords = gpu.getTextureCoordinates(
        Rectangle(0, 0, matrix->getWidth() - 4 * MULT_STAGE_STEPS, matrix->getHeight() - 1),
        IntPoint(matrix->getWidth(), matrix->getHeight()),
        IntPoint(buffer[0]->getWidth() - leftPadding, buffer[0]->getHeight())
    );

    // determine the number of summation iterations
    sumStageDelta.clear();
    sumStageTexCoords.clear();
    bool isLastIteration = false;
    int outBufWidth = buffer[0]->getWidth() - leftPadding;
    const int bufHeightPix = buffer[0]->getHeight();
    while (!isLastIteration) {
        const int iterNum = (int)sumStageDelta.size();
        const int inBufIdx = iterNum % 2;       // intermediate buffer index being read at the current iteration

        // update meaningful output width
        const int inBufWidth = outBufWidth;
        outBufWidth = ceili(outBufWidth, SUM_STAGE_STEPS);
        isLastIteration = (outBufWidth == 1);
#ifdef BEATMUP_DEBUG
        DebugAssertion::check(outBufWidth > 0, "non-positive outBufWidth");
#endif

        // setup second intermediate buffer if not yet
        if (iterNum == 0 && !isLastIteration) {
            delete buffer[1];
            buffer[1] = new Matrix(gpu, outBufWidth + leftPadding, matrix->getMatrixHeight(), !fixedPointStorage);
        }

        // compute deltas
        sumStageDelta.push_back({});
        auto& deltas = sumStageDelta.back();
        for (size_t i = 0; i < deltas.size(); ++i)
            deltas[i] = (float)i / buffer[inBufIdx]->getWidth();

        // compute texture coords
        const int rem = inBufWidth - outBufWidth * SUM_STAGE_STEPS;
        const int ySamplesNum = (isLastIteration && fixedPointStorage && !fixed16Output) ? 2 : 1;
            // number of samples along Y dimension

        sumStageTexCoords.emplace_back(gpu.getTextureCoordinates(
            Rectangle(leftPadding + rem, 0, leftPadding + inBufWidth - SUM_STAGE_STEPS, bufHeightPix - ySamplesNum),
            IntPoint(buffer[inBufIdx]->getWidth(), bufHeightPix),
            IntPoint(outBufWidth, bufHeightPix / ySamplesNum)
        ));

        // deltas[0] is not used; use it here to pass y step
        deltas[0] = 1.0f / buffer[inBufIdx]->getHeight();
    }

    // make summation stage program
    {
        String code(BEATMUP_SHADER_CODE_V(
            varying highp vec2 texCoord;
        ));

        code.printf("uniform sampler2D %s;", UNIFORM_MATRIX);
        code.printf("uniform highp float %s[%d];", UNIFORM_DELTA, sumStageDelta[0].size());
        declareGlsl16bitFixedPointFunctions(code, "", fixedPointStorage, false, fixedPointStorage, 8 + precisionBoost, 8 + precisionBoost);
        code.nl();
        code.line("void main() {");
        if (!fixedPointStorage) {
            code("gl_FragColor = ");
            for (size_t i = 0; i < sumStageDelta[0].size(); ++i)
                if (i > 0)
                    code.printf(" + texture2D(%s, vec2(texCoord.x + %s[%u], texCoord.y))", UNIFORM_MATRIX, UNIFORM_DELTA, i);
                else
                    code.printf("texture2D(%s, texCoord)", UNIFORM_MATRIX);
            code.line(";");
        }
        else {
            for (size_t i = 0; i < sumStageDelta[0].size(); ++i) {
                if (i == 0) {
                    code.printf("lowp vec4 i = texture2D(%s, texCoord);", UNIFORM_MATRIX);
                    code.printf("highp vec2 s = ");
                }
                else {
                    code.printf("i = texture2D(%s, vec2(texCoord.x + %s[%u], texCoord.y));", UNIFORM_MATRIX, UNIFORM_DELTA, i);
                    code("s += ");
                }
                code("vec2(unpack(i[0], i[1]), unpack(i[2], i[3]));");
            }
            code("gl_FragColor = vec4(pack(s.x), pack(s.y));");
        }
        code("}");

        if (bank)
            sumStage = (*bank)(gpu, code);
        else
            sumStage = new RenderingProgram(gpu, FragmentShader(gpu, code));
    }

    // make last iteration summation stage program if needed
    if (fixedPointStorage || bias) {
        String code(BEATMUP_SHADER_CODE_V(
            varying highp vec2 texCoord;
        ));

        code.printf("uniform sampler2D %s;", UNIFORM_MATRIX);
        if (bias)
            code.printf("uniform sampler%dD %s;", VECTOR_TEXTURE_DIMS, UNIFORM_BIAS);
        code.printf("uniform highp float %s[%d];", UNIFORM_DELTA, sumStageDelta[0].size());

        // declare packing/unpacking routines
        declareGlsl16bitFixedPointFunctions(code, "", fixed16Output, false, true, 8, 8 + precisionBoost);
        if (bias)
            declareGlsl16bitFixedPointFunctions(code, "Bias", false, false, true);

        code.line("void main() {");
        if (bias)
            code.line("highp vec2 b;");

        // floating point case first
        if (!fixedPointStorage) {
            if (fixed16Output)
                throw ImplementationUnsupported("16 bit fixed point output is not supported in floating point computing mode");

            code("gl_FragColor = ");
            for (size_t i = 0; i < sumStageDelta[0].size(); ++i)
                if (i > 0)
                    code.printf(" + texture2D(%s, vec2(texCoord.x + %s[%u], texCoord.y))", UNIFORM_MATRIX, UNIFORM_DELTA, i);
                else
                    code.printf("texture2D(%s, texCoord)", UNIFORM_MATRIX);

            if (bias) {
                code.printf(" + texture%dD(%s, vec%d(", VECTOR_TEXTURE_DIMS, UNIFORM_BIAS, VECTOR_TEXTURE_DIMS);
                for (int i = 0; i < VECTOR_TEXTURE_DIMS; ++i) {
                    if (i > 0)
                        code(",");
                    code(i == VECTOR_MAIN_DIM ? "texCoord.y" : "0");
                }
                code("))");
            }
        }

        else {
            for (size_t i = 0; i < sumStageDelta[0].size(); ++i) {
                // sample input
                if (i == 0) {
                    code.printf("lowp vec4 i = texture2D(%s, texCoord);", UNIFORM_MATRIX);
                    if (!fixed16Output)
                        code.printf("lowp vec4 i2 = texture2D(%s, vec2(texCoord.x, texCoord.y + %s[0]));", UNIFORM_MATRIX, UNIFORM_DELTA);
                }
                else {
                    code.printf("i = texture2D(%s, vec2(texCoord.x + %s[%u], texCoord.y));", UNIFORM_MATRIX, UNIFORM_DELTA, i);
                    if (!fixed16Output)
                        code.printf("i2 = texture2D(%s, vec2(texCoord.x + %s[%u], texCoord.y + %s[0]));", UNIFORM_MATRIX, UNIFORM_DELTA, i, UNIFORM_DELTA);
                }

                // sum up
                code(i == 0 ? "highp vec2 s = " : "s += ");
                code.line("vec2(unpack(i[0], i[1]), unpack(i[2], i[3]));");
                if (!fixed16Output) {
                    code(i == 0 ? "highp vec2 s2 = " : "s2 += ");
                    code.line("vec2(unpack(i2[0], i2[1]), unpack(i2[2], i2[3]));");
                }
            }

            // add bias
            if (bias) {
                sampleVectorComponent(code, "", "i", UNIFORM_BIAS, "texCoord.y");
                if (!fixed16Output)
                    sampleVectorComponent(code, "", "i2", UNIFORM_BIAS, "texCoord.y", 0);

                code.line("b = vec2(unpackBias(i[0], i[1]), unpackBias(i[2], i[3]));");
                if (bias->getMappingScale() != 1 || bias->getMappingOffset() != 0)
                    code.printf("b = %0.8f * (b - %0.8f);", 1 / bias->getMappingScale(), bias->getMappingOffset());
                code.line("s += b;");

                if (!fixed16Output) {
                    code.line("b = vec2(unpackBias(i2[0], i2[1]), unpackBias(i2[2], i2[3]));");
                    if (bias->getMappingScale() != 1 || bias->getMappingOffset() != 0)
                        code.printf("b = %0.8f * (b - %0.8f);", 1 / bias->getMappingScale(), bias->getMappingOffset());
                    code.line("s2 += b;");
                }
            }

            code("gl_FragColor = ");
            if (fixed16Output)
                code("vec4(pack(s.x), pack(s.y))");
            else
                code("vec4(s, s2)");
        }

        code.line(";");
        code("}");

        if (bank)
            lastSumStage = (*bank)(gpu, code);
        else
            lastSumStage = new RenderingProgram(gpu, FragmentShader(gpu, code));
    }
    else
        lastSumStage = nullptr;

    ready = true;
}


void LinearMapping::process(GraphicPipeline& gpu, TextureHandler& output, TextureHandler& input) {
    static const int MATRIX_UNIT = 1, INPUT_UNIT = 2, BIAS_UNIT = 3;

    // first stage: compute small 4*4 matrix-by-vector products
    multStage->enable(gpu);
    gpu.bindOutput(*buffer[0]);

    int outBufWidth = buffer[0]->getWidth() - leftPadding;
    glViewport(leftPadding, 0, outBufWidth, buffer[0]->getHeight());

    matrix->bind(gpu, MATRIX_UNIT);
    multStage->setInteger(UNIFORM_MATRIX, MATRIX_UNIT);

    gpu.bind(input, INPUT_UNIT, TextureParam::INTERP_NEAREST);
    multStage->setInteger(UNIFORM_INPUT, INPUT_UNIT);

    multStage->setFloatArray(UNIFORM_DELTA, multStageDelta.data(), multStageDelta.size());
    gpu.setTextureCoordinates(multStageTexCoords);
    multStage->blend();

    // stage 2: sum them up
    sumStage->enable(gpu);
    sumStage->setInteger(UNIFORM_MATRIX, MATRIX_UNIT);
    for (size_t i = 0; i + 1 < sumStageTexCoords.size(); ++i) {
        gpu.bindOutput(*buffer[(i + 1) % 2]);
        outBufWidth = ceili(outBufWidth, SUM_STAGE_STEPS);
        glViewport(leftPadding, 0, outBufWidth, buffer[0]->getHeight());

        buffer[i % 2]->bind(gpu, MATRIX_UNIT);
        sumStage->setFloatArray(UNIFORM_DELTA, sumStageDelta[i].data(), sumStageDelta[i].size());
        gpu.setTextureCoordinates(sumStageTexCoords[i]);
        sumStage->blend();
    }

    // stage 2, last iteration
    RenderingProgram* lastProgram = lastSumStage ? lastSumStage : sumStage;
    if (lastProgram != sumStage) {
        lastProgram->enable(gpu);
        lastProgram->setInteger(UNIFORM_MATRIX, MATRIX_UNIT);
    }
    gpu.bindOutput(output);
    glViewport(0, 0, output.getWidth(), output.getHeight());

    buffer[(sumStageTexCoords.size() - 1) % 2]->bind(gpu, MATRIX_UNIT);
    if (bias) {
        lastProgram->setInteger(UNIFORM_BIAS, BIAS_UNIT);
        gpu.bind(*bias, BIAS_UNIT, TextureParam::INTERP_NEAREST);
    }

    lastProgram->setFloatArray(UNIFORM_DELTA, sumStageDelta.back().data(), sumStageDelta.back().size());
    gpu.setTextureCoordinates(sumStageTexCoords.back());
    lastProgram->blend();
}


void LinearMapping::operator()(GraphicPipeline& gpu, TextureHandler& output, TextureHandler& input) {
    gpu.switchMode(GraphicPipeline::Mode::INFERENCE);
    prepare(gpu, output, input);
    process(gpu, output, input);
}

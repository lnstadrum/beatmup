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

#include "color_matrix.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/processing.h"
#include "../color/color_spaces.h"
#include "../color/constants.h"

using namespace Beatmup;

Filters::ColorMatrix::ColorMatrix() :
    allowIntApprox(true), bias(Color::ZERO_F)
{}


namespace Kernels {
    /**
        Application of a Color::Matrix on CPU
    */
    template <class in_t, class out_t> class ApplyColorMatrix {
    public:
        static void process(AbstractBitmap& input, AbstractBitmap& output, int x, int y, const pixfloat4 &biasF, const Color::Matrix& matrixF, bool useIntApprox, msize nPix) {
            in_t in(input, x, y);
            out_t out(output, x, y);

            if (useIntApprox) {
                pixint4 matrixI[4], biasI;
                biasI = biasF;
                for (int i = 0; i < 4; i++)
                    matrixI[i] = matrixF[i];
                for (msize n = 0; n < nPix; n++) {
                    out.assign(
                        (in() * matrixI[CHANNELS_4.R]).sum() + biasI.val[CHANNELS_4.R],
                        (in() * matrixI[CHANNELS_4.G]).sum() + biasI.val[CHANNELS_4.G],
                        (in() * matrixI[CHANNELS_4.B]).sum() + biasI.val[CHANNELS_4.B],
                        (in() * matrixI[CHANNELS_4.A]).sum() + biasI.val[CHANNELS_4.A]
                    );
                    in++;
                    out++;
                }
            }
            else
                for (msize n = 0; n < nPix; n++) {
                    out.assign(
                        (in() * matrixF[CHANNELS_4.R]).sum() + biasF[CHANNELS_4.R],
                        (in() * matrixF[CHANNELS_4.G]).sum() + biasF[CHANNELS_4.G],
                        (in() * matrixF[CHANNELS_4.B]).sum() + biasF[CHANNELS_4.B],
                        (in() * matrixF[CHANNELS_4.A]).sum() + biasF[CHANNELS_4.A]
                    );
                    in++;
                    out++;
                }
        }
    };
}


void Filters::ColorMatrix::apply(int x, int y, msize nPix, TaskThread& thread) {
    BitmapProcessing::pipeline<Kernels::ApplyColorMatrix>(
        *inputBitmap, *outputBitmap, x, y,
        bias, matrix, allowIntApprox && inputBitmap->isInteger() && outputBitmap->isInteger(),
        nPix
    );
}


std::string Filters::ColorMatrix::getGlslDeclarations() const {
    if (allowIntApprox)
        return
            "uniform lowp mat4 transform;" \
            "uniform lowp vec4 bias;";
    else
        return
            "uniform highp mat4 transform;" \
            "uniform highp vec4 bias;";
}


std::string Filters::ColorMatrix::getGlslSourceCode() const {
    return "gl_FragColor = " + PixelwiseFilter::GLSL_RGBA_INPUT + " * transform + bias;";
}


void Filters::ColorMatrix::setup(bool useGpu) {
    if (useGpu) {
        shader->setFloatMatrix4("transform", matrix);
        shader->setFloat("bias", bias.r, bias.g, bias.b, bias.a);
    }
}


void Filters::ColorMatrix::allowIntegerApproximations(bool allow) {
    allowIntApprox = allow;
}


void Filters::ColorMatrix::setCoefficients(int outChannel, float bias, float r, float g, float b, float a) {
    OutOfRange::check(outChannel, 0, 3, "Invalid output channel index: %d");
    pixfloat4 _(this->bias);
    _[outChannel] = bias;
    this->bias = _;
    matrix[outChannel].r = r;
    matrix[outChannel].g = g;
    matrix[outChannel].b = b;
    matrix[outChannel].a = a;
}


void Filters::ColorMatrix::setHSVCorrection(float hueDegrees, float saturationFactor, float valueFactor) {
    matrix = Color::Matrix(hueDegrees, saturationFactor, valueFactor);
    bias = Color::ZERO_F;
}


void Filters::ColorMatrix::setColorInversion(color3f preservedHue, float saturationFactor, float valueFactor) {
    matrix = Color::Matrix(preservedHue, saturationFactor, valueFactor);
    bias = Color::ZERO_F;
}


void Filters::ColorMatrix::applyContrast(float factor) {
    matrix.r().r *= factor;
    matrix.r().g *= factor;
    matrix.r().b *= factor;
    matrix.g().r *= factor;
    matrix.g().g *= factor;
    matrix.g().b *= factor;
    matrix.b().r *= factor;
    matrix.b().g *= factor;
    matrix.b().b *= factor;
}


void Filters::ColorMatrix::setBrightness(float brightness) {
    bias.r = brightness;
    bias.g = brightness;
    bias.b = brightness;
}
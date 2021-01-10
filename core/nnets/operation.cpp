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

#include "operation.h"

#define POS_FMT "%0.10f"

using namespace Beatmup;
using namespace NNets;

static const char
    *UNIFORM_DELTA = "d",       //!< uniform variable storing spatial deltas for SpatialFilteringMixin
    *UNIFORM_SHIFT = "shift";   //!< uniform variable specifying an offset to apply to the sampling position in SpatialFilteringMixin

const char* SpatialFilteringMixin::SAMPLE_ID_PREFIX = "i";

Size AbstractOperation::getOutputSize(int outputIndex) const {
    throw RuntimeError("Operation " + name + " does not have output #" + std::to_string(outputIndex));
}


Storage::View AbstractOperation::getOutput(int index) {
    throw RuntimeError("Operation " + name + " does not take Storage View on output #" + std::to_string(index));
}


void AbstractOperation::getOutput(GL::Vector*&, int index) {
    throw RuntimeError("Operation " + name + " does not take Vector on output #" + std::to_string(index));
}


void AbstractOperation::getOutput(GL::TextureHandler*&, int index) {
    throw RuntimeError("Operation " + name + " does not take TextureHandler on output #" + std::to_string(index));
}


void AbstractOperation::setInput(Storage::View&&, int index) {
    throw RuntimeError("Operation " + name + " does not take Storage View on input #" + std::to_string(index));
}


void AbstractOperation::setOutput(Storage::View&&, int index) {
    throw RuntimeError("Operation " + name + " does not take Storage View on output #" + std::to_string(index));
}


void AbstractOperation::setInput(GL::Vector&, int index) {
    throw RuntimeError("Operation " + name + " does not take Vector on input #" + std::to_string(index));
}


void AbstractOperation::setOutput(GL::Vector&, int index) {
    throw RuntimeError("Operation " + name + " does not take Vector on output #" + std::to_string(index));
}


void AbstractOperation::setInput(GL::TextureHandler& image, int index) {
    throw RuntimeError("Operation " + name + " does not take TextureHandler on input #" + std::to_string(index));
}


void AbstractOperation::setOutput(GL::TextureHandler&, int index) {
    throw RuntimeError("Operation " + name + " does not take TextureHandler on output #" + std::to_string(index));
}



std::map<std::string, AbstractOperation::Deserializer*>& AbstractOperation::Deserializer::getDeserializersMap() {
    static std::map<std::string, Deserializer*> map;
    return map;
}


AbstractOperation::Deserializer::Deserializer(const char* opType) {
    // add the instance to map
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(getDeserializersMap().count(opType) == 0, "A deserializer of a specific operation is already registered");
#endif
    getDeserializersMap().emplace(opType, this);
}


SpatialFilteringMixin::SpatialFilteringMixin(const int nbSizeX, const int nbSizeY):
    nbSizeX(nbSizeX), nbSizeY(nbSizeY), useUniformShift(false)
{
    deltas = new float[2 * getDeltasSize()];
}


SpatialFilteringMixin::~SpatialFilteringMixin() {
    delete[] deltas;
}


void SpatialFilteringMixin::writeHeader(StringBuilder& code, bool useUniformShift) {
    if (nbSizeX > 1 || nbSizeY > 1)
        code.printf("uniform highp vec2 %s[%d];\n", UNIFORM_DELTA, getDeltasSize());
    if (useUniformShift)
        code.printf("uniform highp vec2 %s;\n", UNIFORM_SHIFT);
    this->useUniformShift = useUniformShift;
}


void SpatialFilteringMixin::declare(StringBuilder& code, const char* datatype, bool inlineSampling) {
    // declaring neighborhood samples
    const int nbSize = std::max(nbSizeX, nbSizeY);
    code.printf("%s i00", datatype);
    for (int i = 1; i < nbSize * nbSize; ++i)
        code.printf(", %s%d%d", SAMPLE_ID_PREFIX, i % nbSize, i / nbSize);
    code.line(";");

    // declaring/computing neighborhood positions
    const int mid = (nbSize - 1) / 2;
    code.printf("highp vec2 p%d = %s", mid, GL::RenderingPrograms::TEXTURE_COORDINATES_ID);
    if (useUniformShift)
        code.printf(" + %s", UNIFORM_SHIFT);
    for (int i = 0; i < nbSize; ++i) {
        if (i != mid) code(", ");
        if (i < mid)
            code.printf("p%d = p%d - %s[%d]", i, mid, UNIFORM_DELTA, mid - i - 1);
        else if (i > mid)
            code.printf("p%d = p%d + %s[%d]", i, mid, UNIFORM_DELTA, i - mid - 1);
    }

    // declaring neighborhood positions shifted for a current input channel
    if (!inlineSampling) {
        code(", cp0");
        for (int i = 1; i < nbSize; ++i)
            code.printf(", cp%d", i);
    }
    code.line(";");
}


void SpatialFilteringMixin::sample(StringBuilder& code, const char* inputName, const int inputIndex, const Point& shift, const bool isFirstSample, const char* suffix) {
    // shift neighborhood positions to the channel origin
    if (isFirstSample || this->shift != shift) {
        const int nbSize = std::max(nbSizeX, nbSizeY);
        for (int i = 0; i < nbSize; ++i)
            if (shift == Point::ZERO)
                code.printf("cp%d = p%d;\n", i, i);
            else
                code.printf("cp%d = p%d + vec2(" POS_FMT ", " POS_FMT ");\n", i, i, shift.x, shift.y);
        this->shift = shift;
    }

    // sample input
    for (int y = 0; y < nbSizeY; ++y)
    for (int x = 0; x < nbSizeX; ++x)
        if (x == y)
            code.printf("%s%d%d = texture2D(%s[%d], cp%d)%s;\n",
                SAMPLE_ID_PREFIX, x, y, inputName, inputIndex, x, suffix);
        else
            code.printf("%s%d%d = texture2D(%s[%d], vec2(cp%d.x, cp%d.y))%s;\n",
                SAMPLE_ID_PREFIX, x, y, inputName, inputIndex, x, y, suffix);
}


void SpatialFilteringMixin::sampleInline(StringBuilder& code, const char* inputName, const int inputIndex, const IntPoint& position, const Point& shift, const char* suffix) {
#ifdef BEATMUP_DEBUG
    OutOfRange::check(position.x, 0, nbSizeX - 1, "neighbor X coordinate is out of range: %d");
    OutOfRange::check(position.y, 0, nbSizeY - 1, "neighbor X coordinate is out of range: %d");
#endif

    // begin texture sampling statement
    code.printf("texture2D(%s[%d], ", inputName, inputIndex);

    // print out texture coordinate
    if (position.x == position.y)
        code.printf("p%d", position.x);
    else
        code.printf("vec2(p%d.x, p%d.y)", position.x, position.y);

    if (shift != Point::ZERO)
        code.printf(" + vec2(" POS_FMT ", " POS_FMT ")", shift.x, shift.y);

    // print the remainder
    code.printf(")%s", suffix);
}


void SpatialFilteringMixin::setup(const int width, const int height) {
    if (nbSizeX > 1 || nbSizeY > 1) {
        const int deltaSize = getDeltasSize();
        for (int i = 0; i < deltaSize; ++i) {
            const float f = (float)(i + 1);
            deltas[2*i  ] = f / width;
            deltas[2*i+1] = f / height;
        }
    }
}


void SpatialFilteringMixin::setUniformShift(GL::Program& program, const IntPoint& shift, const IntPoint& inputSize) {
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(useUniformShift, "Uniform shift is not enabled");
#endif
    program.setVector2(UNIFORM_SHIFT, (float)shift.x / inputSize.x, (float)shift.y / inputSize.y);
}


void SpatialFilteringMixin::setupProgram(GL::Program& program) {
    if (nbSizeX > 1 || nbSizeY > 1)
        program.setVec2Array(UNIFORM_DELTA, deltas, getDeltasSize());
}


IntRectangle SpatialFilteringMixin::getSamplingArea(const IntPoint& size, const IntPoint& stride, const Size::Padding padding) const {
    const IntPoint halfKernel((nbSizeX - 1) / 2, (nbSizeY - 1) / 2);

    if (padding == Size::Padding::VALID) {
        const IntPoint
            // number of times the kernel is applied
            n((size - IntPoint(nbSizeX, nbSizeY)) / stride + 1);

        return IntRectangle(halfKernel, halfKernel + (n - 1) * stride);
    }

    else {
        const IntPoint
            // number of times the kernel is applied
            n(ceili(size,  stride)),

            // pixels participating in all the kernels applications (distance from border to border)
            coverage = (n - 1) * stride + IntPoint(nbSizeX, nbSizeX),

            // excessive pixels to be padded on one side
            ledge = (coverage - size) / 2,

            // the ledge part covered by kernel
            kernelShift = std::min(halfKernel, halfKernel - ledge);

        return IntRectangle(kernelShift, kernelShift + (n - 1) * stride);
    }
}


IntRectangle SpatialFilteringMixin::getSamplingArea(const Storage::View& storage, const int channel, const IntPoint& stride, const Size::Padding padding) const {
    return getSamplingArea(
        IntPoint(storage.getWidth(), storage.getHeight()),
        stride, padding
    ).translated(storage.getChannelOrigin(channel));
}


Rectangle SpatialFilteringMixin::getTextureCoordinates(
    const Storage::View& storage,
    const int channel,
    const IntPoint& stride,
    const Size::Padding padding,
    const IntPoint& outputSize
) const {
    return GraphicPipeline::getTextureCoordinates(
        getSamplingArea(storage, channel, stride, padding),
        IntPoint(storage.getTextureWidth(), storage.getTextureHeight()),
        outputSize
    );
}


std::string SpatialFilteringMixin::getInputSamplingPos() const {
    const int mid = (std::max(nbSizeX, nbSizeY) - 1) / 2;
    return "p" + std::to_string(mid);
}


void ActivationFunctionMixin::apply(StringBuilder& code, const char* inputVariable) {
    switch (activationFunc) {
        case ActivationFunction::DEFAULT:
            code.printf("gl_FragColor = %s;", inputVariable);
            return;
        case ActivationFunction::BRELU6:
            code.printf("gl_FragColor = 0.167 * %s;", inputVariable);
            return;
        default: Insanity::insanity("Invalid activation function");
    }
}


void CpuOperation::execute(TaskThread& thread, GraphicPipeline& gpu) {
    const int num = thread.numThreads();
    beforeExecute(gpu, num);
    const int amount = getAmountOfWork();
    thread.synchronize();

    // process the first hunk
    execute(0, amount / num, 0, num);

    thread.synchronize();
    afterExecute(num);
}


void CpuOperation::execute(TaskThread& thread) {
    thread.synchronize();
    if (thread.isTaskAborted())
        return;
    const int
        amount = getAmountOfWork(),
        idx = thread.currentThread(),
        num = thread.numThreads();
    execute(
         idx      * amount / num,
        (idx + 1) * amount / num,
        idx, num
    );
    thread.synchronize();
}


ActivationFunction NNets::activationFunctionFromString(const std::string& str) {
    const std::string lc = StringUtils::lowercase(str);
    if (lc == "default")
        return ActivationFunction::DEFAULT;
    if (lc == "brelu6")
        return ActivationFunction::BRELU6;
    throw InvalidArgument("Invalid activation function: " + str);
    return ActivationFunction::DEFAULT;
}


std::string std::to_string(ActivationFunction function) {
    /** \page NNetsActivationFunctionsSerialization Activation functions serialization
        In the serialized representation, activation functions are specified with a single keyword.
         - `default`: identity clipped to 0..1 range.
         - `brelu6`: 0.167 times identity clipped to 0..1 range.
    */
    switch (function) {
        case ActivationFunction::DEFAULT: return "default";
        case ActivationFunction::BRELU6: return "brelu6";
    }
    Insanity::insanity("Invalid activation function");
    return "";
}
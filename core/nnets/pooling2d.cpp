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

#include "pooling2d.h"
#include "deserialized_model.h"

using namespace Beatmup;
using namespace NNets;


static const char *UNIFORM_INPUT = "features";


Pooling2D::Pooling2D(
    const std::string& name,
    const Operator op,
    const int size,
    const int stride,
    const Size::Padding padding
):
    AbstractOperation(name),
    SpatialFilteringMixin(size, size),
    size(size, size, 1), stride(stride <= 0 ? this->size : Size(stride, stride, 1)),
    op(op), padding(padding),
    ready(false), program(nullptr)
{
    if (op == Operator::AVERAGE)
        InvalidArgument::check(padding == Size::Padding::VALID, "Average 2D pooling only supports valid padding");
    OutOfRange::checkMin(size, 2, "Pooling size must be at least 2, %d got");
}


void Pooling2D::prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank) {
    // delete the old program
    if (program)
        bank.release(gpu, program);

    String code(gpu.getGlslVersionHeader());
    code(GL::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG);

    code.printf("uniform sampler2D %s[1];", UNIFORM_INPUT);
    SpatialFilteringMixin::writeHeader(code, true);

    // main
    code.line("void main() {");

    // declare neighborhood, inline sampling
    SpatialFilteringMixin::declare(code, "lowp vec4", true);

    // declare the result variable
    switch (op) {
        case Operator::MAX:
            code("lowp");
        break;
        case Operator::AVERAGE:
            code("mediump");
        break;
    }
    code.line(" vec4 r;");

    // compute pooling
    switch (op) {
        case Operator::MAX:
            for (int i = 0; i < size.volume(); ++i) {
                code(i == 0 ? "r = (" : "r = max(r, ");
                SpatialFilteringMixin::sampleInline(code, UNIFORM_INPUT, 0, IntPoint(i % size[0], i / size[0]), Point::ZERO);
                code.line(");");
            }
            code.line("gl_FragColor = r;");
        break;
        case Operator::AVERAGE:
            code("r = ");
            for (int i = 0; i < size.volume(); ++i) {
                if (i > 0)
                    code(" + ");
                SpatialFilteringMixin::sampleInline(code, UNIFORM_INPUT, 0, IntPoint(i % size[0], i / size[0]), Point::ZERO);
            }
            code.line(";");
            code.printf("gl_FragColor = %0.10f * r;", 1.0f / size.volume());
        break;
    }

    // store the result
    code("}");

    // init program
    program = bank(gpu, code);

    ready = true;
}


void Pooling2D::execute(TaskThread& thread, GraphicPipeline& gpu) {
    if (!ready)
        throw NotReady(this);
    RuntimeError::check(input, "Input is not provided to a Pooling2D operation.");
    RuntimeError::check(output, "Output is not provided to Pooling2D operation " + getName());
    RuntimeError::check(input.getDepth() == output.getDepth(), "Input / output depth mismatch.");

    // prepare deltas
    program->enable(gpu);
    SpatialFilteringMixin::setup(input.getTextureWidth(), input.getTextureHeight());
    SpatialFilteringMixin::setupProgram(*program);

    // setup texture coordinates
    gpu.setTextureCoordinates(
        getSamplingArea(input, 0, IntPoint(stride[0], stride[1]), padding),
        input.getTextureSize(),
        output.getSpatialSize()
    );

    // for each output channel
    const IntPoint origin = input.getChannelOrigin(0);
    Storage::Binder bind(gpu);
    for (int channel = 0; channel < input.getDepth(); channel += 4) {
        // bind things to program
        bind.begin(*program, output, channel);
        bind(input, UNIFORM_INPUT, channel);

        // setup
        SpatialFilteringMixin::setUniformShift(*program, input.getChannelOrigin(channel) - origin, input.getTextureSize());

        // run
        program->blend();
    }
}


int Pooling2D::getInputPadding(int index) const {
    return (padding == Size::Padding::SAME) ? std::max(size[0], size[1]) / 2 : 0;
}


void Pooling2D::getSampledChannels(int index, int& min, int& max) const {
    min = max = (index == 0 ? 4 : 0);
}


Size Pooling2D::getOutputSize(int outputIndex) const {
    if (outputIndex == 0) {
        RuntimeError::check(input, "Input is not provided to a Pooling2D operation.");
        const Size result = input.getSize().transform(size, stride, padding, input.getDepth());
        RuntimeError::check(result.volume() > 0, "Invalid (zero or negative) output size got in " + getName());
        return result;
    }
    return Size::EMPTY;
}


void Pooling2D::setInput(Storage::View&& view, int inputIndex) {
    OutOfRange::check(inputIndex, 0, 0, "Input index out of range: %d");
    RuntimeError::check(view.getStorage().getPadding() >= getInputPadding(inputIndex), "The storage has insufficient padding");
    this->input = std::move(view);
}


void Pooling2D::setOutput(Storage::View&& storage, int outputIndex) {
    OutOfRange::check(outputIndex, 0, 0, "Output index out of range: %d");
    this->output = std::move(storage);
}


std::map<std::string, std::string> Pooling2D::serialize() const {
    return {
        { "_name",      getName() },
        { "_type",      "pooling2d" },
        { "operator",   std::to_string(op) },
        { "size",       std::to_string(size[0]) },
        { "stride",     std::to_string(stride[0]) },
        { "padding",    std::to_string(padding) }
    };
}


bool Pooling2D::initDeserializer() {
    static class Pooling2DDeserializer : public AbstractOperation::Deserializer {
    public:
        Pooling2DDeserializer() : Deserializer("pooling2d") {}
        AbstractOperation* deserialize(Context& context, const Listing::Block& block) {
            /** \page NNetsOpsSerialization
                \section Pooling2D
                \code{yaml}
                - _name: arbitrary operation name
                  _type: pooling2d     # fixed string
                  operator: max        # "max" or "average"
                  size: 3              # pooling size
                  stride: 2            # stride (defaults to 1)
                  padding: valid       # paddling, string, "valid" or "same" (defaults to "valid")
                \endcode
            */
            return new Pooling2D(
                block["_name"],
                Pooling2D::operatorFromString(block["operator"]),
                block.get<int>("size"),
                block.get<int>("stride", 1),
                paddingFromString(block.get<std::string>("padding", std::to_string(Size::Padding::VALID)))
            );
        }
    } john;

    return true;
}


void Pooling2D::disconnect() {
    this->input = Storage::View();
    this->output = Storage::View();
}


Pooling2D::Operator Pooling2D::operatorFromString(const std::string& str) {
    const std::string lc = StringUtils::lowercase(str);
    if (lc == "average")
        return Operator::AVERAGE;
    if (lc == "max")
        return Operator::MAX;
    throw InvalidArgument("Invalid pooling operator: " + str);
    return Operator::AVERAGE;
}


std::string std::to_string(Pooling2D::Operator op) {
    switch (op) {
        case Pooling2D::Operator::AVERAGE: return "average";
        case Pooling2D::Operator::MAX: return "max";
    }
    Insanity::insanity("Invalid pooling operator");
    return "";
}
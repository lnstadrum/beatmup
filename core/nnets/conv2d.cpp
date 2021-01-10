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

#include "../exception.h"
#include "conv2d.h"
#include "deserialized_model.h"
#include <algorithm>

using namespace Beatmup;
using namespace NNets;


// hardcoded coefficient and coordinate formats
#define COEF_FMT "%0.6f"
#define COORD_FMT "%0.10f"

const char *Conv2D::FILTERS_CHUNK_SUFFIX = "/w";
const char *Conv2D::BIAS_CHUNK_SUFFIX = "/b";

static const char
    *UNIFORM_INPUT = "features",
    *UNIFORM_RESIDUAL_INPUT = "residual",
    *UNIFORM_COEFFICIENT = "cc";


Conv2D::Conv2D(
    const std::string& name,
    const int kernelSize,
    const int numInputChannels,
    const int numOutputChannels,
    const int stride,
    const Size::Padding padding,
    const bool useBias,
    const int numGroups,
    const ActivationFunction activation
):
    AbstractOperation(name), SpatialFilteringMixin(kernelSize, kernelSize), ActivationFunctionMixin(activation),
    kernelSize(kernelSize, kernelSize, numInputChannels / numGroups), numOutputChannels(numOutputChannels), numGroups(numGroups),
    stride(stride), padding(padding),
    useInputImage(numInputChannels == 3),
    isDepthwise(numInputChannels == numGroups && numOutputChannels == numGroups),
    useBias(useBias),
    ready(false),
    inputImage(nullptr)
{
    if (useInputImage) {
        InvalidArgument::check(numGroups == 1, "Cannot apply a group convolution to the input image");
        InvalidArgument::check(padding == Size::Padding::VALID, "Only valid zero padding setting is supported when an image is used as input");
    }
    else
        Storage::checkChannelNumber(numInputChannels);
    Storage::checkChannelNumber(numOutputChannels);
    OutOfRange::checkMin(stride, 1, "Positive convolution stride expected, %d got");
    OutOfRange::checkMin(kernelSize, 1, "Positive convolution kernel size expected, %d got");
    if (!useInputImage && !isDepthwise)
        InvalidArgument::check(this->kernelSize.getDepth() % 4 == 0, "A multiple of 4 is expected as number of input channels in the convolution kernel.");
    if (!isDepthwise && numGroups > 1)
        OutOfRange::checkMin(this->kernelSize.getDepth(), 4, "Kernels having less than 4 input channels are not supported in grouped convolutions. Got %d channels.");

    // check groups alignment: each group must contain 4k inputs and outputs channels
    if (!isDepthwise) {
        if (!useInputImage)
            InvalidArgument::check(numInputChannels % (4 * numGroups) == 0,
                "Cannot split " +std::to_string(numInputChannels)+ " input channels on " +std::to_string(numGroups)+ " groups of 4*k channels each.");
        InvalidArgument::check(numOutputChannels % (4 * numGroups) == 0,
            "Cannot split " +std::to_string(numOutputChannels)+ " output channels on " +std::to_string(numGroups)+ " groups of 4*k channels each.");
    }
    programs.reserve(numOutputChannels / 4);
    groupViews.reserve(numGroups);
}


void Conv2D::prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank) {
    RuntimeError::check((useInputImage && inputImage) || (!useInputImage && input), "Input is not provided to Conv2D operation " + getName());
    RuntimeError::check(output, "Output is not provided to Conv2D operation " + getName());

    // get coefficients
    const Chunk kernel(data, getName() + FILTERS_CHUNK_SUFFIX);
    if (kernel.size() != kernelSize.volume() * numOutputChannels * sizeof(float))
        throw InconsistentModelData(this, "Weights size mismatch");

    const Chunk* biases = nullptr;
    if (useBias) {
        biases = new Chunk(data, getName() + BIAS_CHUNK_SUFFIX);
        if (biases->size() != numOutputChannels * sizeof(float))
            throw InconsistentModelData(this, "Biases size mismatch");
    }

    // free old stuff
    for (auto program : programs)
        bank.release(gpu, program);
    programs.clear();
    coeffs.clear();

    // decide whether use uniforms or not
    static const int MAX_ALLOWED_NUMBER_OF_PROGRAMS = 0;    // discovered empirically that uniforms are faster on Pi, Nano and desktop
    static const int NUM_RESERVED_UNFORM_VECTORS = 8 + std::max(kernelSize[0], kernelSize[1]) / 2;     // number of uniform vectors to keep unused
    const int numberOfPrograms = numOutputChannels / 4;
    const int uniformsLength = kernelSize.volume() + 1;     // number of uniform vectors per program
    const bool useUniforms = !useInputImage &&                  // if an image is given on input, the uniforms use is not unsupported
        numberOfPrograms > MAX_ALLOWED_NUMBER_OF_PROGRAMS &&    // if not too many programs, rather go with hardcoded model data
        uniformsLength + NUM_RESERVED_UNFORM_VECTORS < gpu.getLimit(GraphicPipeline::Limit::FRAGMENT_UNIFORM_VECTORS);
    if (useUniforms)
        coeffs.reserve(numberOfPrograms * uniformsLength);

    const bool useUniformShift = useUniforms && kernelSize.getDepth() <= 4;
        //!< use uniform shift if only one input texture is sampled, i.e., depthwise or grouped with groups of 4

    // init new programs
    for (int outputChannel = 0; outputChannel < numOutputChannels; outputChannel += 4) {
        const size_t coefStart = coeffs.size();     // index of the first coefficient in coeffs for the current program

        // compute indices delimiting the current group
        const int groupIdx = outputChannel * numGroups / numOutputChannels;
        const int firstInputChannel = groupIdx * kernelSize.getDepth();
        const int lastInputChannel  = firstInputChannel + (isDepthwise ? 4 : kernelSize.getDepth());

        // set up GLSL code
        String code(BEATMUP_SHADER_HEADER_VERSION);
        code(GL::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG);

#ifdef BEATMUP_DEBUG
        if (!groupViews.empty())
            DebugAssertion::check(groupViews.back().getNumberOfTextures() <= gpu.getLimit(GraphicPipeline::Limit::TEXTURE_IMAGE_UNITS),
                "Cannot compute Conv2D operation " + getName() + " on the current GPU: too many textures per group");
#endif

        code.printf("uniform sampler2D %s[%d];", UNIFORM_INPUT, useInputImage || isDepthwise ? 1 : groupViews[groupIdx].getNumberOfTextures());
        if (residualInput)
            code.printf("uniform sampler2D %s[1];", UNIFORM_RESIDUAL_INPUT);
        if (useUniforms)
            code.printf("uniform highp vec4 %s[%d];", UNIFORM_COEFFICIENT, uniformsLength);

        SpatialFilteringMixin::writeHeader(code, useUniformShift);
        code.line("void main() {");
        code.line("highp vec4 sum;");

        // declare neighborhood: vec4 for storage, vec3 for image
        SpatialFilteringMixin::declare(code, useInputImage ? "highp vec3" : "highp vec4", !useInputImage);

        // loop through input channels
        for (int inputChannel = firstInputChannel; inputChannel < lastInputChannel; inputChannel += 4) {
            const int channelInGroup = inputChannel - firstInputChannel;

            const Point shift = (useUniformShift || !input) ? Point::ZERO :
                (Point(input.getChannelOrigin(inputChannel) - input.getChannelOrigin(firstInputChannel)) / input.getTextureSize());
                // texture coordinates sample the first channel in the current group, so shift is relative to its origin

            // compute depthwise convolution: inline sampling used
            if (isDepthwise) {
                code("sum = ");
                for (int y = 0; y < kernelSize[1]; ++y)
                for (int x = 0; x < kernelSize[0]; ++x) {
                    if (x > 0 || y > 0) code(" + ");
                    const float* w = kernel.ptr<float>(getIdx(outputChannel, 0, x, y));
                    if (useUniforms) {
                        code.printf("%s[%d] * ", UNIFORM_COEFFICIENT, (int)(coeffs.size() - coefStart));
                        coeffs.emplace_back(std::array<float, 4>{ w[0], w[1], w[2], w[3] });
                    }
                    else
                        code.printf("vec4(" COEF_FMT "," COEF_FMT "," COEF_FMT "," COEF_FMT ") * ", w[0], w[1], w[2], w[3]);
                    SpatialFilteringMixin::sampleInline(code, UNIFORM_INPUT, 0, IntPoint(x, y), shift);
                }
                code.line(";");
            }

            // compute convolution with 3-channel input image using dot product; no inline sampling
            else if (useInputImage) {
                SpatialFilteringMixin::sample(code, UNIFORM_INPUT, 0, Point::ZERO, true, useInputImage ? ".rgb" : "");
                const int offset[4] = { 0, 1 * numOutputChannels, 2 * numOutputChannels, 3 * numOutputChannels };
                for (int y = 0; y < kernelSize[1]; ++y)
                for (int x = 0; x < kernelSize[0]; ++x) {
                    code((channelInGroup == 0 && x == 0 && y == 0) ? "sum = vec4(" : "sum += vec4(");
                    for (int c = 0; c < 4; ++c) {
                        if (c > 0) code(",");
                        const float* w = kernel.ptr<float>(getIdx(c + outputChannel, channelInGroup, x, y));
                        code.printf("dot(vec3(" COEF_FMT "," COEF_FMT "," COEF_FMT "), %s%d%d)",
                                w[0], w[offset[1]], w[offset[2]], SpatialFilteringMixin::SAMPLE_ID_PREFIX, x, y);
                    }
                    code.line(");");
                }
            }

            // compute 4m to 4n channels using vector by 4x4 matrix multiply: inline sampling used
            else {
                code.printf("sum %s", channelInGroup == 0 ? "=" : "+=");
                const int offset[4] = { 0, 1 * numOutputChannels, 2 * numOutputChannels, 3 * numOutputChannels };
                for (int y = 0; y < kernelSize[1]; ++y)
                for (int x = 0; x < kernelSize[0]; ++x) {
                    if (x > 0 || y > 0) code(" + ");
                    SpatialFilteringMixin::sampleInline(code, UNIFORM_INPUT, groupViews[groupIdx].getChannelTextureNumber(channelInGroup), IntPoint(x, y), shift);
                    code.printf(" * mat4(");
                    for (int c = 0; c < 4; ++c) {
                        if (c > 0) code(",");
                        const float* w = kernel.ptr<float>(getIdx(c + outputChannel, channelInGroup, x, y));
                        if (useUniforms) {
                            code.printf("%s[%d]", UNIFORM_COEFFICIENT, (int)(coeffs.size() - coefStart));
                            coeffs.emplace_back(std::array<float, 4>{ w[0], w[offset[1]], w[offset[2]], w[offset[3]] });
                        }
                        else
                            code.printf(COEF_FMT "," COEF_FMT "," COEF_FMT "," COEF_FMT, w[0], w[offset[1]], w[offset[2]], w[offset[3]]);
                    }
                    code.printf(")");
                }
                code.line(";");
            }
        }

        // add residual input
        if (residualInput) {
            // get linear mapping of channel pixel positions to sample the residual input properly
            const IntPoint mainOrigin = input.getChannelOrigin(useUniformShift ? outputChannel : firstInputChannel);
            const IntPoint residualOrigin = residualInput.getChannelOrigin(outputChannel);
            const Rectangle mainArea(mainOrigin, mainOrigin + input.getSpatialSize());
            const Rectangle resArea(residualOrigin, residualOrigin + residualInput.getSpatialSize());
            const Point mainTexSize(input.getTextureWidth(), input.getTextureHeight());
            const Point resTexSize(residualInput.getTextureWidth(), residualInput.getTextureHeight());
            Point scale, offset;
            (mainArea / mainTexSize).getMapping(resArea / resTexSize, scale, offset);
            // sample, add to sum
            code.printf("sum += texture2D(%s[0], %s * vec2(" COORD_FMT "," COORD_FMT ") + vec2(" COORD_FMT "," COORD_FMT "));\n",
                UNIFORM_RESIDUAL_INPUT, getInputSamplingPos().c_str(), scale.x, scale.y, offset.x, offset.y);
        }

        // add bias if enabled
        if (useBias) {
            const float* b = biases->ptr<float>(outputChannel);
            if (useUniforms) {
                code.printf("sum += %s[%d];", UNIFORM_COEFFICIENT, (int)(coeffs.size() - coefStart));
                coeffs.emplace_back(std::array<float, 4>{ b[0], b[1], b[2], b[3] });
            }
            else
                code.printf("sum += vec4(" COEF_FMT "," COEF_FMT "," COEF_FMT "," COEF_FMT ");\n", b[0], b[1], b[2], b[3]);
        }

        // apply activation
        ActivationFunctionMixin::apply(code, "sum");
        code("}");

        // init program
        programs.push_back(bank(gpu, code));
    }

    // setup execution order: same programs writing to the same texture are next to each other
    execOrder.resize(programs.size());
    for (size_t i = 0; i < execOrder.size(); ++i)
        execOrder[i] = (int)i;
    std::sort(execOrder.begin(), execOrder.end(), [&](int i, int j) {
        return programs[i] < programs[j] || (programs[i] == programs[j] &&
            output.getChannelTextureNumber(4 * i) < output.getChannelTextureNumber(4 * j));
    });

    delete biases;
    ready = true;
}


void Conv2D::execute(TaskThread& thread, GraphicPipeline& gpu) {
    if (!ready)
        throw NotReady(this);

    RuntimeError::check((useInputImage && inputImage) || (!useInputImage && input), "Input is not provided to a Conv2D operation.");
    RuntimeError::check(output, "Output is not provided to Conv2D operation " + getName());
    if (residualInput && residualInput.getSize() != output.getSize())
        throw RuntimeError("Residual input size does not match the output size");

#ifdef BEATMUP_DEBUG
    RuntimeError::check(output.getSize() == getOutputSize(), "Operation output storage size mismatch");
#endif

    // static program setup
    SpatialFilteringMixin::setup(
        useInputImage ? inputImage->getWidth()  : input.getTextureWidth(),
        useInputImage ? inputImage->getHeight() : input.getTextureHeight()
    );

    // compute tex coords
    const IntPoint strides(stride, stride);
    const IntPoint inputTextureSize = useInputImage ?
        IntPoint(inputImage->getWidth(), inputImage->getHeight()) :
        IntPoint(input.getTextureWidth(), input.getTextureHeight());
    if (useInputImage || isUniformShiftUsed()) {
        const IntRectangle samplingArea = useInputImage ?
            getSamplingArea(IntPoint(inputImage->getWidth(), inputImage->getHeight()), strides, padding) :
            getSamplingArea(input, 0, strides, padding);

        gpu.setTextureCoordinates(samplingArea, inputTextureSize, output.getSpatialSize());
    }

    const int coeffsPerProgram = (int)(coeffs.size() / programs.size());
    const bool uniformsAreUsed = coeffsPerProgram > 0;

    // for each output channel
    Storage::Binder bind(gpu);
    for (size_t i = 0; i < execOrder.size(); ++i) {
        const int programNum = execOrder[i];
        const int outputChannel = 4 * programNum;

        GL::RenderingProgram& program = *programs[programNum];

        if (isDepthwise) {
            const int channel = outputChannel;

            // bind output to a program
            const bool fast = bind.begin(program, output, outputChannel);

            if (!fast) {
                // bind inputs
                bind(input, UNIFORM_INPUT, outputChannel);
                if (residualInput)
                    bind(residualInput, UNIFORM_RESIDUAL_INPUT, outputChannel);
                SpatialFilteringMixin::setupProgram(program);
            }

            // setup the remaining stuff
            if (isUniformShiftUsed())
                SpatialFilteringMixin::setUniformShift(program, input.getChannelOrigin(channel) - input.getChannelOrigin(0), input.getTextureSize());
            else
                gpu.setTextureCoordinates(getSamplingArea(input, channel, strides, padding), inputTextureSize, output.getSpatialSize());
        }

        else {
            // bind output to a program
            const int groupIdx = outputChannel * numGroups / numOutputChannels;
            const bool isSameGroup =  i > 0 && 4 * execOrder[i - 1] * numGroups / numOutputChannels == groupIdx;
            const bool fast = bind.begin(program, output, outputChannel) && isSameGroup;

            const int firstInputChannel = groupIdx * kernelSize.getDepth();
            const int lastInputChannel  = firstInputChannel + (isDepthwise ? 4 : kernelSize.getDepth());

            if (!fast) {
                // bind inputs
                if (useInputImage)
                    bind(*inputImage, UNIFORM_INPUT);
                else {
                    const int firstInputChannel = groupIdx * kernelSize.getDepth();
                    bind(groupViews[groupIdx], UNIFORM_INPUT);

                    if (residualInput)
                        bind(residualInput, UNIFORM_RESIDUAL_INPUT, outputChannel);

                    if (isUniformShiftUsed())
                        SpatialFilteringMixin::setUniformShift(program, input.getChannelOrigin(firstInputChannel) - input.getChannelOrigin(0), input.getTextureSize());
                    else
                        gpu.setTextureCoordinates(getSamplingArea(input, firstInputChannel, strides, padding), inputTextureSize, output.getSpatialSize());
                }

                // setup the remaining stuff
                SpatialFilteringMixin::setupProgram(program);
            }
        }

        // update uniforms if needed
        if (uniformsAreUsed)
            program.setVec4Array(UNIFORM_COEFFICIENT, coeffs[coeffsPerProgram * programNum].data(), coeffsPerProgram);

        // g-g-go
        program.blend();
    }
}


int Conv2D::getInputPadding(int index) const {
    return (index == 0 && padding == Size::Padding::SAME) ? std::max(kernelSize[0], kernelSize[1]) / 2 : 0;
}


void Conv2D::getSampledChannels(int index, int& min, int& max) const {
    if (index == 0) {
        // main input: sampling an entire group at once
        min = useInputImage ? 3 : 4;
        max = useInputImage ? 3 : isDepthwise ? 4 : kernelSize.getDepth();
    }
    else if (index == 1) {
        // residual input: sampling 1 texture at once
        min = max = 4;
    }
    else
        min = max = 0;
}


Size Conv2D::getOutputSize(int outputIndex) const {
    if (outputIndex == 0) {
        RuntimeError::check((useInputImage && inputImage) || (!useInputImage && input),
            "Input is not provided to Conv2D operation " + getName());
        const Size inputSize = useInputImage ? Size(inputImage->getWidth(), inputImage->getHeight(), 3) : input.getSize();
        const Size result = inputSize.transform(
            kernelSize,
            Size(stride, stride, 0),
            padding,
            numOutputChannels
        );
        RuntimeError::check(result.volume() > 0, "Invalid (zero or negative) output size got in " + getName());
        return result;
    }
    return Size::EMPTY;
}


std::map<std::string, std::string> Conv2D::serialize() const {
    return {
        { "_name",              getName() },
        { "_type",              "conv2d" },
        { "kernel_size",        std::to_string(kernelSize[0]) },
        { "input_channels",     std::to_string(kernelSize.getDepth() * numGroups) },
        { "output_channels",    std::to_string(numOutputChannels) },
        { "stride",             std::to_string(stride) },
        { "padding",            std::to_string(padding) },
        { "use_bias",           useBias ? "true" : "false" },
        { "groups",             std::to_string(numGroups) },
        { "activation",         std::to_string(activationFunc) }
    };
}


bool Conv2D::initDeserializer() {
    static class Conv2DDeserializer : public AbstractOperation::Deserializer {
    public:
        Conv2DDeserializer() : Deserializer("conv2d") {}
        AbstractOperation* deserialize(Context& context, const Listing::Block& block) {
            /** \page NNetsOpsSerialization Operations serialization
                This page describes the operation options.

                Every operation necessary has `_name` and `_type` parameters. The rest of operation parameters depends on its type.

                \section Conv2D
                \code{yaml}
                - _name: arbitrary operation name
                  _type: conv2d        # fixed string
                  kernel_size: 3       # size of convolution kernel
                  input_channels: 3    # number of input feature channels
                  output_channels: 16  # number of output feature channels
                  stride: 2            # stride (defaults to 1)
                  padding: valid       # paddling, string, "valid" or "same" (defaults to "valid")
                  use_bias: true       # bias addition, "true" or "false" (defaults to "true")
                  groups: 1            # number of groups for grouped convolution (defaults to 1)
                  activation: default  # activation function
                \endcode

                For activation functions see \ref NNetsActivationFunctionsSerialization.
            */
            return new Conv2D(
                block["_name"],
                block.get<int>("kernel_size"),
                block.get<int>("input_channels"),
                block.get<int>("output_channels"),
                block.get<int>("stride", 1),
                paddingFromString(block.get<std::string>("padding", std::to_string(Size::Padding::VALID))),
                block.get<bool>("use_bias", true),
                block.get<int>("groups", 1),
                activationFunctionFromString(block.get<std::string>("activation", std::to_string(ActivationFunction::DEFAULT)))
            );
        }
    } john;

    return true;
}

void Conv2D::disconnect() {
    inputImage = nullptr;
    input = Storage::View();
    residualInput = Storage::View();
    output = Storage::View();
    groupViews.clear();
}


void Conv2D::setInput(Storage::View&& view, int inputIndex) {
    OutOfRange::check(inputIndex, 0, 1, "Input index out of range: %d");
    RuntimeError::check(view.getStorage().getPadding() >= getInputPadding(inputIndex), "The storage has insufficient padding");
    if (inputIndex == 0) {
        if (view) {
            RuntimeError::check(!useInputImage, "An image is expected on input, but a tensor is passed");
            RuntimeError::check(view.getDepth() == kernelSize.getDepth() * numGroups, "Tensor depth does not match kernel depth");
            // create group views
            groupViews.clear();
            if (!isDepthwise)
                for (int groupIdx = 0; groupIdx < numGroups; ++groupIdx) {
                    const int firstInputChannel = groupIdx * kernelSize.getDepth();
                    const int lastInputChannel  = firstInputChannel + (isDepthwise ? 4 : kernelSize.getDepth());
                    groupViews.emplace_back(std::move(view), firstInputChannel, lastInputChannel - firstInputChannel);
                }
        }
        this->input = std::move(view);
        this->inputImage = nullptr;
    }
    else {
        if (view) {
            RuntimeError::check(!useInputImage, "Cannot use the residual input when an image is used as the main input");
            RuntimeError::check(view.getDepth() == numOutputChannels, "Residual input tensor depth does not match output depth");
        }
        this->residualInput = std::move(view);
    }
}


void Conv2D::setOutput(Storage::View&& storage, int outputIndex) {
    OutOfRange::check(outputIndex, 0, 0, "Output index out of range: %d");
    this->output = std::move(storage);
}


void Conv2D::setInput(GL::TextureHandler& image, int inputIndex) {
    if (inputIndex == 0) {
        RuntimeError::check(useInputImage, "Cannot use image as Conv2D input");
        this->inputImage = &image;
    }
    else
        AbstractOperation::setInput(image, inputIndex);
}


unsigned long Conv2D::countMultiplyAdds() const {
    return getOutputSize(0).volume() * kernelSize.volume();
}
#include "operation.h"
#include "../../gpu/tensor.h"
#include "../../bitmap/internal_bitmap.h"
#include "../../utils/string_builder.h"
using namespace Beatmup;
using namespace NNets;
const Size Size::ONES(1, 1, 1);
Size::Size(int width, int height, int depth) {
    dim[0] = width;
    dim[1] = height;
    dim[2] = depth;
}
Size Size::transform(Size kernel, Size stride, Padding padding, int depth) const {
    Size result(dim[0], dim[1], depth == 0 ? this->dim[2] : depth);
    if (padding == Padding::SAME) {
        result.dim[0] = ceili(dim[0], stride[0]);
        result.dim[1] = ceili(dim[1], stride[1]);
        if (depth == 0)
            result.dim[2] = ceili(dim[2], stride[2]);
    }
    else {
        result.dim[0] = ceili(dim[0] - kernel[0] + 1, stride[0]);
        result.dim[1] = ceili(dim[1] - kernel[1] + 1, stride[1]);
        if (depth == 0)
            result.dim[2] = ceili(dim[2] - kernel[2] + 1, stride[2]);
    }
    return result;
}
Size Size::getOrigin(Size kernel, Size stride, Padding padding) const {
    if (padding == Padding::SAME) {
        return Size(
            kernel[0] / 2 - (kernel[0] - ((dim[0] - 1) % stride[0]) - 1) / 2,
            kernel[1] / 2 - (kernel[1] - ((dim[1] - 1) % stride[1]) - 1) / 2,
            kernel[2] / 2 - (kernel[2] - ((dim[2] - 1) % stride[2]) - 1) / 2
        );
    }
    else {
        return Size(kernel[0] / 2, kernel[1] / 2, kernel[2] / 2);
    }
}
template<> std::string Size::toString<2>() const {
    static const size_t BUF_SIZE = 28;
    char buf[BUF_SIZE];
#ifdef _MSC_VER
    sprintf_s
#else
    snprintf
#endif
        (buf, BUF_SIZE, "%d,%d", dim[0], dim[1]);
    return buf;
}
template<> std::string Size::toString<3>() const {
    static const size_t BUF_SIZE = 40;
    char buf[BUF_SIZE];
#ifdef _MSC_VER
    sprintf_s
#else
    snprintf
#endif
        (buf, BUF_SIZE, "%d,%d,%d", dim[0], dim[1], dim[2]);
    return buf;
}
void AbstractOperation::TypeMismatch::check(Storage::Type expected, Storage::Type got) {
    if (expected != got)
        throw TypeMismatch(expected, got);
}
AbstractOperation::AbstractOperation(const std::string& name) :
    name(name)
{}
Storage::Type AbstractOperation::getInputType(int inputIndex) const {
    return Storage::getDefaultType(getInputSize(inputIndex));
}
Storage::Type AbstractOperation::getOutputType(int outputIndex) const {
    return Storage::getDefaultType(getOutputSize(outputIndex));
}
bool AbstractOperation::setInputType(Storage::Type type, int inputIndex) {
    // by default the default type is accepted only
    return type == getInputType(inputIndex);
}
AbstractOperation::~AbstractOperation() {}
std::string AbstractOperation::getProgramChunkId(size_t idx) const {
    return getName() + "#" + std::to_string(idx);
}
void AbstractOperation::store(GraphicPipeline& gpu, ChunkFile::Writer& cache, ProgressTracking& progress) {
    // nothing to do by default
}
void AbstractOperation::load(GraphicPipeline& gpu, ChunkFile& cache, ProgressTracking& progress) {
    // nothing to do by default
}
int AbstractOperation::getInputCount() const {
    return 1;
}
int AbstractOperation::getOutputCount() const {
    return 1;
}
GPUOperation::GPUOperation(Context& ctx, const std::string& name) :
    AbstractOperation(name), ctx(ctx)
{}
GPUOperation::~GPUOperation() {
    for (auto& program: programs)
        if (program)
            program->destroy(*ctx.getGpuRecycleBin());
}
void GPUOperation::store(GraphicPipeline& gpu, ChunkFile::Writer& cache, ProgressTracking& progress) {
    if (programs.empty())
        throw NotReady(this);
    BEATMUP_ASSERT_DEBUG(getMaxProgress() == programs.size());
    // write out number of programs
    cache(getName(), programs.size());
    // write out program binaries
    for (size_t i = 0; i < programs.size(); ++i) {
        if (programs[i]) {
            Chunk* binary = programs[i]->getBinary();
            binary->writeTo(cache, getProgramChunkId(i));
            delete binary;
        }
        progress();
    }
}
void GPUOperation::load(GraphicPipeline& gpu, ChunkFile& cache, ProgressTracking& progress) {
    // free existing programs
    for (auto& program : programs)
        if (program)
            program->destroy(*ctx.getGpuRecycleBin());
    programs.clear();
    const size_t numPrograms = cache.fetch<size_t>(getName());
    programs.resize(numPrograms, nullptr);
    BEATMUP_ASSERT_DEBUG(numPrograms == getMaxProgress());
    for (size_t i = 0; i < numPrograms; ++i) {
        const std::string chunkId = getProgramChunkId(i);
        if (cache.chunkExists(chunkId)) {
            Chunk binary(cache, getProgramChunkId(i));
            programs[i] = new GL::Object<GL::ComputeProgram>(gpu);
            programs[i]->loadBinary(binary);
            progress();
        }
    }
}
void GPUOperation::perform(GraphicPipeline& gpu) {
    for (int i = 0; i < (int)programs.size(); ++i) {
        BEATMUP_ASSERT_DEBUG(programs[i]);
        perform(gpu, *programs[i], i);
    }
}
void GPUOperation::prepare(GraphicPipeline& gpu, ChunkFile& data, ProgressTracking& progress) {
    // grab source code for every program
    std::vector<std::string> sourceCodes = generateCode(gpu, data);
    programs.resize(sourceCodes.size());
    BEATMUP_ASSERT_DEBUG(getMaxProgress() == programs.size());
    // make programs
    for (size_t i = 0; i < sourceCodes.size(); ++i) {
        if (!programs[i])
            programs[i] = new GL::Object<GL::ComputeProgram>(gpu);
        programs[i]->make(gpu, sourceCodes[i]);
        progress();
    }
}
Storage* GPUOperation::allocateOutput(int outputIndex) const {
    BEATMUP_ASSERT_DEBUG(0 <= outputIndex);
    BEATMUP_ASSERT_DEBUG(outputIndex < getOutputCount());
    const Size size = getOutputSize(outputIndex);
    const Storage::Type type = getOutputType(outputIndex);
    return new Storage(ctx, size.getWidth(), size.getHeight(), size.getDepth(), type);
}
Ops::ImageInput::ImageInput(Context& ctx, const std::string& name, const Size& size) :
    GPUOperation(ctx, name), input(nullptr), output(nullptr), size(size)
{}
void Ops::ImageInput::setInput(Storage& storage, int inputIndex) {
    BEATMUP_ASSERT_DEBUG(inputIndex == 0);
    TypeMismatch::check(Storage::Type::TEXTURE_REFERENCE, storage.getType());
    input = &storage;
}
void Ops::ImageInput::setOutput(Storage& storage, int outputIndex) {
    BEATMUP_ASSERT_DEBUG(outputIndex == 0);
    BEATMUP_ASSERT_DEBUG(storage.getWidth() == size.getWidth() && storage.getHeight() == size.getHeight() && storage.getDepth() == 4);
    output = &storage;
}
Storage* Ops::ImageInput::allocateOutput(int outputIndex) const {
    BEATMUP_ASSERT_DEBUG(outputIndex == 0);
    return new Storage(ctx, size.getWidth(), size.getHeight(), 4, getOutputType());
}
Storage::Type Ops::ImageInput::getOutputType(int outputIndex) const {
    return Storage::Type::TENSOR_8FP_SIGNED;
}
std::vector<std::string> Ops::ImageInput::generateCode(GraphicPipeline& gpu, ChunkFile& data) {
    Storage::CodeGenerator gen;
    gen("outp", getOutputType(), size, 1, "writeonly", Storage::CodeGenerator::Access::STORING);
    std::vector<std::string> code(1, BEATMUP_SHADER_CODE_V(
        layout(local_size_x = 1, local_size_y = 1) in;
        layout(binding = 0) uniform highp sampler2D inputImage;
        $OUT$
        uniform highp int width;
        uniform highp int height;
        void main() {
            store(ivec3(gl_GlobalInvocationID.xy, 0),
                texture(inputImage, vec2(gl_GlobalInvocationID.xy) / vec2(width, height)).rgba +
                    - vec4(0.5, 0.5, 0.5, 0.5)
            );
        }
    ));
    StringBuilder builder(code[0]);
    builder.replace("$OUT$", gen.get());
    return code;
}
void Ops::ImageInput::perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int workerIdx) const {
    BEATMUP_ASSERT_DEBUG(input);
    BEATMUP_ASSERT_DEBUG(output);
    program.enable(gpu);
    Size outputSize(getOutputSize());
    program.setInteger("width", outputSize.getWidth());
    program.setInteger("height", outputSize.getHeight());
    input->bind(gpu, program, 0, true, false);
    output->bind(gpu, program, 1, false, true);
    program.dispatch(gpu, outputSize.getWidth(), outputSize.getHeight(), 1);
}
FeedforwardGPUOperation::FeedforwardGPUOperation(
    Context& ctx,
    const std::string& name,
    const Size& inputSize,
    const int outChannelsNum
) :
    GPUOperation(ctx, name),
    outChannelsNum(outChannelsNum),
    outChanNumRoundUp4(4 * ceili(outChannelsNum, 4)),
    inputType(Storage::getDefaultType(inputSize)),
    inputSize(inputSize),
    input(nullptr), output(nullptr)
{}
Storage::Type FeedforwardGPUOperation::getInputType(int inputIndex) const {
    return inputType;
}
Size FeedforwardGPUOperation::getInputSize(int inputIndex) const {
    BEATMUP_ASSERT_DEBUG(inputIndex == 0);
    return inputSize;
}
void FeedforwardGPUOperation::setInput(Storage& storage, int inputIndex) {
    BEATMUP_ASSERT_DEBUG(inputIndex == 0);
    if (inputSize.getDepth() >= 4)
        BEATMUP_ASSERT_DEBUG(storage.getSize() == inputSize);
    input = &storage;
}
void FeedforwardGPUOperation::setOutput(Storage& storage, int inputIndex) {
    BEATMUP_ASSERT_DEBUG(inputIndex == 0);
    output = &storage;
}
void FeedforwardGPUOperation::perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int workerIdx) const {
    BEATMUP_ASSERT_DEBUG(input);
    BEATMUP_ASSERT_DEBUG(output);
    program.enable(gpu);
    input->bind(gpu, program, 0, true, false);
    output->bind(gpu, program, 1, false, true);
}
Feedforward2DGPUOperation::Feedforward2DGPUOperation(
    Context& ctx,
    const std::string& name,
    const Size& inputSize,
    const int outChannelsNum,
    const Size& kernelSize,
    const Size& stride,
    const Size::Padding& padding
) :
    FeedforwardGPUOperation(ctx, name, inputSize, outChannelsNum),
    kernelSize(kernelSize), stride(stride), padding(padding)
{}
Size Feedforward2DGPUOperation::getOutputSize(int outputIndex) const {
    BEATMUP_ASSERT_DEBUG(outputIndex == 0);
    return inputSize.transform(kernelSize, stride, padding, outChannelsNum);
}Probe::Probe(Context& ctx, const std::string& name, const Size& size, int laneIndex) :
    GPUOperation(ctx, name), input(nullptr), size(size), laneIndex(laneIndex)
{
    BEATMUP_ASSERT_DEBUG(laneIndex * 4 < size.getDepth());
    output = new InternalBitmap(ctx, PixelFormat::QuadFloat, size.getWidth(), size.getHeight());
}
Probe::~Probe() {
    delete output;
}
void Probe::setInput(Storage& storage, int inputIndex) {
    BEATMUP_ASSERT_DEBUG(inputIndex == 0);
    BEATMUP_ASSERT_DEBUG(storage.getWidth() == size.getWidth() && storage.getHeight() == size.getHeight());
    input = &storage;
}
bool Probe::setInputType(Storage::Type type, int inputIndex) {
    if (inputIndex != 0)
        return false;
    inputType = type;
    return true;
}
void Probe::setOutput(Storage& storage, int outputIndex) {
    Insanity::insanity("Probe output cannot be set up explicitly");
}
Storage* Probe::allocateOutput(int outputIndex) const {
    Insanity::insanity("Probe output cannot be allocated explicitly");
    return nullptr;
}
std::vector<std::string> Probe::generateCode(GraphicPipeline& gpu, ChunkFile& data) {
    std::vector<std::string> code(1, BEATMUP_SHADER_CODE_V(
        layout(local_size_x = 1, local_size_y = 1) in;
        $IN$
        layout(binding = 1, rgba32f) uniform highp writeonly image2D outp;
        uniform int channel;
        vec4 map(vec4 i) {
            return vec4(i.rgb + 0.5, 1);
        }
        void main() {
            imageStore(outp, ivec2(gl_GlobalInvocationID.xy), map(load(ivec3(gl_GlobalInvocationID.xy, channel))));
        }
    ));
    Storage::CodeGenerator accessCode;
    accessCode("inp", inputType, getInputSize(), 0, "readonly", Storage::CodeGenerator::Access::LOADING);
    StringBuilder builder(code[0]);
    builder.replace("$IN$", accessCode.get());
    return code;
}
void Probe::perform(GraphicPipeline& gpu, GL::ComputeProgram& program, int workerNum) const {
    BEATMUP_ASSERT_DEBUG(input);
    BEATMUP_ASSERT_DEBUG(output);
    program.enable(gpu);
    input->bind(gpu, program, 0, true, false);
    gpu.bind(*output, 1, false, true);
    program.setInteger("channel", laneIndex);
    Size size(getOutputSize());
    program.dispatch(gpu, size.getWidth(), size.getHeight(), 1);
    output->lockPixels(ProcessingTarget::CPU);
    gpu.fetchPixels(*output);
    output->unlockPixels();}
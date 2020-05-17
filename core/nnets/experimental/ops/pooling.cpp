#include "pooling.h"
#include "../../../utils/string_builder.h"

#define STRINGIFY(X) #X

using namespace Beatmup;
using namespace NNets;
using namespace Ops;

        
Pooling::Pooling(
    Context& ctx,
    const std::string& name,
    const Mode mode,
    const Size& inputSize,
    const int outChannelsNum,
    const Size& kernelSize,
    const Size& stride,
    Size::Padding padding
):
    Feedforward2DGPUOperation(ctx, name, inputSize, outChannelsNum, kernelSize, stride, padding),
    mode(mode)
{
    if (mode == Mode::AVERAGE && padding != Size::Padding::VALID)
        throw InconsistentModelData(this, "Invalid average pooling padding: VALID only is supported.");
}


bool Pooling::setInputType(Storage::Type type, int index) {
    if (index == 0 && (type == Storage::Type::TENSOR
        || type == Storage::Type::TENSOR_8FP_BRELU6
        || type == Storage::Type::TENSOR_16FP_BRELU6
    )) {
        inputType = type;
        return true;
    }
    else
        return false;
}


std::vector<std::string> Pooling::generateCode(GraphicPipeline& gpu, ChunkFile& data) {
    std::string computeCode;
    StringBuilder computeCodeBuild(computeCode);

    // declare inputs & outputs
    Storage::CodeGenerator accessCode;
    accessCode
        ("inputTensor",  getInputType(),  getInputSize(),  0, "readonly",  Storage::CodeGenerator::Access::LOADING)
        ("outputTensor", getOutputType(), getOutputSize(), 1, "writeonly", Storage::CodeGenerator::Access::STORING);
    
    if (mode == Mode::AVERAGE) {
        const int
            W = kernelSize[0],
            H = kernelSize[1];

        computeCodeBuild.printf(STRINGIFY(
            vec4 sum = vec4(0, 0, 0, 0);
            for (int x = %d; x < %d; ++x)
                for (int y = %d; y < %d; ++y)
                    sum += load(ivec3(P0 + ivec2(x, y), gl_GlobalInvocationID.z));
            store(ivec3(gl_GlobalInvocationID), sum / %d.0f);
        ), -W/2, -W/2 + W, -H/2, -H/2 + H, W*H);
    }

    else
        Insanity::insanity("mode not implemented");

    std::vector<std::string> code(1, BEATMUP_SHADER_CODE_V(
        layout(local_size_x = 1, local_size_y = 1) in;
        <<DECL>>
        void main() {
            mediump ivec2 P0 = ivec2(gl_GlobalInvocationID.xy) * ivec2(<<STR>>) + ivec2(<<OFF>>);
            <<CC>>
            memoryBarrier();
        }
    ));

    StringBuilder builder(code[0]);
    builder
        .replace("<<DECL>>", accessCode.get())
        .replace("<<STR>>", stride.toString<2>())
        .replace("<<OFF>>", inputSize.getOrigin(kernelSize, stride, padding).toString<2>())
        .replace("<<CC>>", computeCode);
    
    return code;
}


void Pooling::perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int sliceIdx) const {
    Feedforward2DGPUOperation::perform(gpu, program, sliceIdx);
    const Size out(inputSize.transform(kernelSize, stride, padding, outChannelsNum));
    program.dispatch(gpu, out[0], out[1], out[2]);
}

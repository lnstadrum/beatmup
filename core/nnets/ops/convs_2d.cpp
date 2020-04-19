#include "convs_2d.h"
#include "../../gpu/tensor.h"
#include "../../utils/bitset.h"
#include "../../debug.h"


#define STRINGIFY(A) #A
#define FMT_W "%0.9f"	//!< format when outputting a conv weight coefficient in GLSL
#define FMT_B "%0.12f"	//!< format when outputting a conv bias coefficient in GLSL


using namespace Beatmup;
using namespace NNets;
using namespace Ops;


/**
    Storage buffers binding points in shaders
*/
static const int
    GLSL_BINDING_WEIGHTS = 2,
    GLSL_BINDING_BIASES = 3;


namespace std {
    template<typename datatype> inline datatype mmin(datatype arg1, datatype arg2, datatype arg3) {
        return std::min(arg1, std::min(arg2, arg3));
    }

    template<typename datatype> inline datatype mmin(datatype arg1, datatype arg2, datatype arg3, datatype arg4) {
        return std::min(std::min(arg1, arg2), std::min(arg3, arg4));
    }
}


Convolution2D::ComputingMode Convolution2D::chooseMode(Size kernel, int outputChannels, bool depthwise) {
    // pointwise first
    if (kernel[0] == 1 && kernel[1] == 1 && kernel[2] % 4 == 0)
        // fixme: "% 4"
        return ComputingMode::BUFFERS;

    if (depthwise)
        return outputChannels <= 256*4 ? ComputingMode::INLINE : ComputingMode::BUFFERS;

    return outputChannels < 256 ? ComputingMode::INLINE : ComputingMode::ARRAYS;
}


Convolution2D::Convolution2D(
    Context& ctx,
    const std::string& name,
    const Size& inputSize,
    const int outChannelsNum,
    const Size& kernelSize,
    bool depthwise,
    const Size& stride,
    Size::Padding padding,
    ActivationFunc actFunc
) :
    Feedforward2DGPUOperation(ctx, name, inputSize, outChannelsNum, kernelSize, stride, padding),
    mode(chooseMode(kernelSize, outChannelsNum, depthwise)),
    actFunc(actFunc),
    depthwise(depthwise),
    weights(*ctx.getGpuRecycleBin()),
    biases(*ctx.getGpuRecycleBin())
{}


Convolution2D::~Convolution2D() {}


const int Convolution2D::getNumberOfSlices() const {
    if (mode == ComputingMode::BUFFERS)
        return 1;

    static const int MAX_OPERATIONS_PER_SLICE = 2048;
    const int numOps = kernelSize.volume() * outChannelsNum;
    return std::min<int>(outChannelsNum / 4, ceili(numOps, MAX_OPERATIONS_PER_SLICE));
}


Storage::Type Convolution2D::getOutputType(int outputIndex) const {
    if (actFunc == ActivationFunc::BRELU6)
        return Storage::Type::TENSOR_16FP_BRELU6;
    return AbstractOperation::getOutputType(outputIndex);
}


bool Convolution2D::setInputType(Storage::Type type, int index) {
    if (index == 0) {
        inputType = type;
        return true;
    }
    else
        return false;
}


int Convolution2D::prepareInputPrefetching(const GraphicPipeline& gpu, StringBuilder& dataCode, Size& localGroup,
    int minScalarZLayers, int maxZThreads)
{
    /*
                  |           ^           |
        <-pad0/2->|         pad1/2        |
                  |           v           |
        ----------+-----------------------+----------   ^
                  |                       |             :
                  :       Convolution     :             :
                  :        operating      :            size
                  :          area         :             :
                  |                       |             :
        ----------+-----------------------+----------   v
                  |                       |
                  |                       |
                  |                       |
    */

    const int
        pad0 = kernelSize[0] / 2 * 2,
        pad1 = kernelSize[1] / 2 * 2,
        limit = gpu.getLimit(GraphicPipeline::Limit::SHARED_MEM) / (Storage::getEntrySize(inputType) * ceili(minScalarZLayers, 4));

    // solving (size + pad0) * (size + pad1) < limit
    const int
        _b = (pad0 + pad1),
        _c = pad0 * pad1 - limit,
        disc = _b * _b - 4 * _c;

    BEATMUP_ASSERT_DEBUG(disc >= 0);

    const int size = std::min<int>(
        (-_b + std::sqrt(_b * _b - 4 * _c)) / 2,
        std::min(inputSize[0], inputSize[1])
    );

    BEATMUP_ASSERT_DEBUG(size > 0);

    localGroup[2] = std::mmin(maxZThreads,
        ceili(minScalarZLayers, 4),
        gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_Z));

    localGroup[0] = std::min(std::min(size / stride[0],
        (int)std::sqrt(gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_TOTAL) / localGroup[2])),
        gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_X));

    localGroup[1] = std::min(std::min(size / stride[1],
        (int)std::sqrt(gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_TOTAL) / localGroup[2])),
        gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_Y));

    // compute work area size
    Size workAreaSize((localGroup - 1) * stride + 1 + (kernelSize / 2) * 2);

    // check for Z expanison: if there are enough unused shared memory, increase workgroup size along Z
    if (localGroup[2] < maxZThreads) {
        const int
            usedSharedMemSize = workAreaSize[0] * workAreaSize[1] * Storage::getEntrySize(inputType),
            zExp = std::mmin(
                gpu.getLimit(GraphicPipeline::Limit::SHARED_MEM) / usedSharedMemSize,
                gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_Z),
                gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_TOTAL) / (localGroup[0] * localGroup[1]),
                maxZThreads);
        localGroup[2] = zExp;
    }

    // declare shared memory
    dataCode.printf("shared highp %s sharedInput[%d][%d][%d];", Storage::getTypeName(inputType).c_str(), workAreaSize[0], workAreaSize[1], ceili(minScalarZLayers, 4));

    return workAreaSize[0];
}


void Convolution2D::renderDepthwise(const GraphicPipeline& gpu, const int sliceIdx, StringBuilder& computeCode, StringBuilder& dataCode) {
    // pick a slice of output channels
    const int
        numSlices = getNumberOfSlices(),
        slice[2] = {
            outChannelsNum * sliceIdx / numSlices / 4 * 4,
            outChannelsNum * (sliceIdx + 1) / numSlices / 4 * 4
        },
        sliceSize = slice[1] - slice[0];

    // prefetch
    const int workAreaSize = prepareInputPrefetching(gpu, dataCode, localGroupSize, sliceSize, sliceSize / 4);
    computeCode.printf<512>(STRINGIFY(
        mediump ivec2 o = ivec2(gl_WorkGroupID * gl_WorkGroupSize) * ivec2(<<STR>>) + ivec2(<<OFF>>) - ivec2(%d, %d);
        for (uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
            i < %du;
            i += gl_WorkGroupSize.x * gl_WorkGroupSize.y)
        {
            ivec2 p = ivec2(i %% %du, i / %du);
            sharedInput[p.x][p.y][gl_LocalInvocationID.z] = loadPacked(ivec3(o + p, gl_GlobalInvocationID.z));
        }

        memoryBarrierShared();
        barrier();
    ),
        kernelSize[0] / 2, kernelSize[1] / 2,
        workAreaSize*workAreaSize, workAreaSize, workAreaSize);

    // compute
    computeCode.printf<512>(STRINGIFY(
        vec4 buf = b[gl_GlobalInvocationID.z];
        mediump ivec2 oo = ivec2(gl_LocalInvocationID.xy) * ivec2(<<STR>>);
        for (int y = 0; y < %d; ++y)
            for (int x = 0; x < %d; ++x) {
                buf += unpack(sharedInput[oo.x + x][oo.y + y][gl_LocalInvocationID.z]) * weightLoad(uvec4(x, y, 0, gl_GlobalInvocationID.z));
            }
        store(dpos(int(gl_GlobalInvocationID.z)), actf(buf));
    ),
        kernelSize[1], kernelSize[0]);
}


void Convolution2D::renderDepthwiseArrayBased(
    const GraphicPipeline& gpu,
    const int sliceIdx,
    StringBuilder& computeCode,
    StringBuilder& dataCode,
    Array<float, 4>& kernels,
    Array<float, 1>& biases
) {
    const int
        numSlices = getNumberOfSlices(),
        X = kernelSize[0],
        Y = kernelSize[1],
        Z = kernelSize[2];

    // pick a slice of output channels
    const int
        slice[2] = {
            outChannelsNum * sliceIdx / numSlices / 4 * 4,
            outChannelsNum * (sliceIdx + 1) / numSlices / 4 * 4
        },
        sliceSize = slice[1] - slice[0];

    // write out weights
    dataCode.printf("vec4 W[%d][%d][%d] = vec4[%d][%d][%d](",
        ceili(sliceSize, 4), Y, X,
        ceili(sliceSize, 4), Y, X);
    for (int c = slice[0]; c < slice[1]; c += 4) {
        dataCode.printf("vec4[%d][%d](", Y, X);
        for (int y = 0; y < Y; ++y) {
            dataCode.printf("vec4[%d](", X);
            for (int x = 0; x < X; ++x) {
                dataCode.printf("vec4(" FMT_W "," FMT_W "," FMT_W "," FMT_W ")%s",
                    kernels(c, 0, x, y),
                    c + 1 >= slice[1] ? 0 : kernels(c + 1, 0, x, y),
                    c + 2 >= slice[1] ? 0 : kernels(c + 2, 0, x, y),
                    c + 3 >= slice[1] ? 0 : kernels(c + 3, 0, x, y),
                    x + 1 < X ? "," : "");
            }
            dataCode.printf(")%s", y + 1 < Y ? "," : "");
        }
        dataCode.printf(")%s", c + 4 < slice[1] ? "," : "");
    }
    dataCode(");");

    // write out biases
    dataCode.printf("vec4 B[%d] = vec4[%d](",
        ceili(sliceSize, 4),
        ceili(sliceSize, 4));
    for (int c = slice[0]; c < slice[1]; c += 4) {
        dataCode.printf("vec4(" FMT_B "," FMT_B "," FMT_B "," FMT_B ")%s",
            biases(c),
            c + 1 >= slice[1] ? 0 : biases(c + 1),
            c + 2 >= slice[1] ? 0 : biases(c + 2),
            c + 3 >= slice[1] ? 0 : biases(c + 3),
            c + 4 < slice[1] ? "," : "");
    }
    dataCode(");");

    // prefetch
    const int workAreaSize = prepareInputPrefetching(gpu, dataCode, localGroupSize, sliceSize, sliceSize / 4);
    computeCode.printf<512>(STRINGIFY(
        int c = int(gl_GlobalInvocationID.z) + %d;
        mediump ivec2 o = ivec2(gl_WorkGroupID * gl_WorkGroupSize) * ivec2(<<STR>>) + ivec2(<<OFF>>) - ivec2(%d, %d);
        for (uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
            i < %du;
            i += gl_WorkGroupSize.x * gl_WorkGroupSize.y)
        {
            ivec2 p = ivec2(i %% %du, i / %du);
            sharedInput[p.x][p.y][gl_LocalInvocationID.z] = loadPacked(ivec3(o + p, c));
        }

        memoryBarrierShared();
        barrier();
    ),
        slice[0] / 4,
        kernelSize[0] / 2, kernelSize[1] / 2,
        workAreaSize*workAreaSize, workAreaSize, workAreaSize);

    // compute
    computeCode.printf<1024>(STRINGIFY(
        vec4 buf = B[gl_GlobalInvocationID.z];
        mediump ivec2 oo = ivec2(gl_LocalInvocationID.xy) * ivec2(<<STR>>);
        for (int y = 0; y < %d; ++y)
            for (int x = 0; x < %d; ++x) {
                buf += unpack(sharedInput[oo.x + x][oo.y + y][gl_LocalInvocationID.z]) * W[gl_GlobalInvocationID.z][y][x];
            }
        store(dpos(c), actf(buf));
    ),
        kernelSize[1], kernelSize[0]);
}


void Convolution2D::renderDepthwiseInline(
    const GraphicPipeline& gpu,
    const int sliceIdx,
    StringBuilder& computeCode,
    StringBuilder& dataCode,
    Array<float, 4>& kernels,
    Array<float, 1>& biases
) {
    const int
        numSlices = getNumberOfSlices(),
        X = kernelSize[0],
        Y = kernelSize[1],
        Z = kernelSize[2];

    // pick a slice of output channels
    const int
        slice[2] = {
            outChannelsNum * sliceIdx / numSlices / 4 * 4,
            outChannelsNum * (sliceIdx + 1) / numSlices / 4 * 4
        },
        sliceSize = slice[1] - slice[0];

    // prefetch
    const int workAreaSize = prepareInputPrefetching(gpu, dataCode, localGroupSize, sliceSize, 1);

    computeCode.printf<768>(STRINGIFY(
        int c = int(gl_GlobalInvocationID.z) + %d;
        mediump ivec2 o = ivec2(gl_WorkGroupID * gl_WorkGroupSize) * ivec2(<<STR>>) + ivec2(<<OFF>>) - ivec2(%d, %d);
        for (uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
            i < %du;
            i += gl_WorkGroupSize.x * gl_WorkGroupSize.y)
        {
            ivec2 p = ivec2(i %% %du, i / %du);
            ivec2 ap = o + p;
            vec4 f = vec4(1, 1, 1, 1);
            for (int z = 0; z < %d; z++)
                sharedInput[p.x][p.y][z] = loadPacked(ivec3(ap, z + %d));
        }

        memoryBarrierShared();
        barrier();
    ),
        slice[0] / 4,
        kernelSize[0] / 2, kernelSize[1] / 2,
        workAreaSize*workAreaSize, workAreaSize, workAreaSize,
        sliceSize / 4, slice[0] / 4);

    // declare the packetization buffer
    computeCode("highp vec4 buf;");
    computeCode("mediump ivec2 oo = ivec2(gl_LocalInvocationID.xy) * ivec2(<<STR>>);");

    // go through filters
    for (int c = slice[0]; c < slice[1]; c += 4) {
        computeCode.printf("buf = vec4(" FMT_B ", " FMT_B ", " FMT_B ", " FMT_B ") ",
            biases(c),
            c + 1 >= slice[1] ? 0 : biases(c + 1),
            c + 2 >= slice[1] ? 0 : biases(c + 2),
            c + 3 >= slice[1] ? 0 : biases(c + 3)
        );

        for (int y = 0; y < Y; ++y)
            for (int x = 0; x < X; ++x) {
                computeCode.printf("+ unpack(sharedInput[oo.x + %d][oo.y + %d][%d]) * vec4(" FMT_W "," FMT_W "," FMT_W "," FMT_W ")",
                    x,
                    y,
                    (c - slice[0]) / 4,
                    kernels(c, 0, x, y),
                    c + 1 >= slice[1] ? 0 : kernels(c + 1, 0, x, y),
                    c + 2 >= slice[1] ? 0 : kernels(c + 2, 0, x, y),
                    c + 3 >= slice[1] ? 0 : kernels(c + 3, 0, x, y));
            }

        computeCode(";").nl().printf("store(dpos(%d), actf(buf)); ", c / 4);
    }
}


void Convolution2D::renderArrayBased(
    const GraphicPipeline& gpu,
    const int sliceIdx,
    StringBuilder& computeCode,
    StringBuilder& dataCode,
    Array<float, 4>& kernels,
    Array<float, 1>& biases
) const {
    const int
        numSlices = getNumberOfSlices(),
        X = kernelSize[0],
        Y = kernelSize[1],
        Z = kernelSize[2];

    // pick a slice of output channels
    const int
        slice[2] = {
            outChannelsNum * sliceIdx / numSlices / 4 * 4,
            outChannelsNum * (sliceIdx + 1) / numSlices / 4 * 4
        },
        sliceSize = slice[1] - slice[0];

    // write out weights
    dataCode.printf("vec4 W[%d][%d][%d][%d] = vec4[%d][%d][%d][%d](",
        sliceSize, ceili(Z, 4), Y, X,
        sliceSize, ceili(Z, 4), Y, X);
    for (int c = slice[0]; c < slice[1]; c++) {
        dataCode.printf("vec4[%d][%d][%d](", ceili(Z, 4), Y, X);
        for (int z = 0; z < Z; z += 4) {
            dataCode.printf("vec4[%d][%d](", Y, X);
            for (int y = 0; y < Y; ++y) {
                dataCode.printf("vec4[%d](", X);
                for (int x = 0; x < X; ++x) {
                    dataCode.printf("vec4(" FMT_W "," FMT_W "," FMT_W "," FMT_W ")%s",
                        kernels(c, z, x, y),
                        z + 1 >= Z ? 0 : kernels(c, z + 1, x, y),
                        z + 2 >= Z ? 0 : kernels(c, z + 2, x, y),
                        z + 3 >= Z ? 0 : kernels(c, z + 3, x, y),
                        x + 1 < X ? "," : "");
                }
                dataCode.printf(")%s", y + 1 < Y ? "," : "");
            }
            dataCode.printf(")%s", z + 4 < Z ? "," : "");
        }
        dataCode.printf(")%s", c + 1 < slice[1] ? "," : "");
    }
    dataCode(");");

    // write out biases
    dataCode.printf("vec4 B[%d] = vec4[%d](", ceili(sliceSize, 4), ceili(sliceSize, 4));
    for (int c = slice[0]; c < slice[1]; c += 4) {
        dataCode.printf("vec4(" FMT_B "," FMT_B "," FMT_B "," FMT_B ")%s",
            biases(c),
            c + 1 >= slice[1] ? 0 : biases(c + 1),
            c + 2 >= slice[1] ? 0 : biases(c + 2),
            c + 3 >= slice[1] ? 0 : biases(c + 3),
            c + 4 < slice[1] ? "," : "");
    }
    dataCode(");");

    computeCode.printf<1024>(STRINGIFY(
        int c = int(gl_GlobalInvocationID.z);
        vec4 acc[4] = vec4[4](vec4(B[c][0], 0, 0, 0), vec4(B[c][1], 0, 0, 0), vec4(B[c][2], 0, 0, 0), vec4(B[c][3], 0, 0, 0));
        for (int z = 0; z < %d; ++z)
            for (int y = 0; y < %d; ++y)
                for (int x = 0; x < %d; ++x) {
                    vec4 s = src(P0, x - %d, y - %d, z);
                    acc[0] += s * W[4*c+0][z][y][x];
                    acc[1] += s * W[4*c+1][z][y][x];
                    acc[2] += s * W[4*c+2][z][y][x];
                    acc[3] += s * W[4*c+3][z][y][x];
                }
        store(dpos(c + %d), actf(vec4(
            acc[0].x + acc[0].y + acc[0].z + acc[0].w,
            acc[1].x + acc[1].y + acc[1].z + acc[1].w,
            acc[2].x + acc[2].y + acc[2].z + acc[2].w,
            acc[3].x + acc[3].y + acc[3].z + acc[3].w
        )));
    ),
        ceili(Z, 4), Y, X, X / 2, Y / 2, slice[0] / 4);
}


void Convolution2D::renderInline(
    const GraphicPipeline& gpu,
    const int sliceIdx,
    StringBuilder& computeCode,
    StringBuilder& dataCode,
    Array<float, 4>& kernels,
    Array<float, 1>& biases
) {
    const int
        numSlices = getNumberOfSlices(),
        X = kernelSize[0],
        Y = kernelSize[1],
        Z = kernelSize[2];

    // pick a slice of output channels
    const int
        slice[2] = {
            outChannelsNum * sliceIdx / numSlices / 4 * 4,
            outChannelsNum * (sliceIdx + 1) / numSlices / 4 * 4
        },
        sliceSize = slice[1] - slice[0];

    // prefetch
    const int workAreaSize = prepareInputPrefetching(gpu, dataCode, localGroupSize, inputSize[2], 1);

    computeCode.printf<512>(STRINGIFY(
        mediump ivec2 o = ivec2(gl_WorkGroupID * gl_WorkGroupSize) * ivec2(<<STR>>) + ivec2(<<OFF>>) - ivec2(%d, %d);
        for (uint i = gl_LocalInvocationIndex;
            i < %du;
            i += gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z)
        {
            ivec2 p = ivec2(i %% %du, i / %du);
            for (int z = 0; z < %d; z++)
                sharedInput[p.x][p.y][z] = loadPacked(ivec3(o + p, z));
        }

        memoryBarrierShared();
        barrier();
    ),
        kernelSize[0] / 2, kernelSize[1] / 2,
        workAreaSize*workAreaSize, workAreaSize, workAreaSize,
        ceili(inputSize[2], 4));

    // declare the packetization buffer and the accumulator
    computeCode("highp vec4 buf, acc;");
    computeCode("mediump ivec2 oo = ivec2(gl_LocalInvocationID.xy) * ivec2(<<STR>>);");

    // go through filters
    const Size outputSize = getOutputSize();
    for (int c = slice[0]; c < slice[1]; ++c) {
        if (c > slice[0] && c % 4 == 0)
            // if the buffer is filled, flush it out
            computeCode.printf("if (gl_GlobalInvocationID.x < %du && gl_GlobalInvocationID.y < %du) store(dpos(%d), actf(buf));",
                outputSize[0], outputSize[1], c / 4 - 1);
        // begin accumulation
        computeCode("acc = ");
        for (int z = 0; z < Z; z += 4)
        for (int y = 0; y < Y; ++y)
        for (int x = 0; x < X; ++x) {
            computeCode.printf("+ unpack(sharedInput[oo.x + %d][oo.y + %d][%d]) * vec4(" FMT_W "," FMT_W "," FMT_W "," FMT_W ")",
                x,
                y,
                z / 4,
                kernels(c, z, x, y),
                z + 1 >= Z ? 0 : kernels(c, z + 1, x, y),
                z + 2 >= Z ? 0 : kernels(c, z + 2, x, y),
                z + 3 >= Z ? 0 : kernels(c, z + 3, x, y));
        }
        computeCode(";").nl()
            // flatten the accumulator
            .printf("buf[%d] = acc.x + acc.y + acc.z + acc.w + " FMT_B ";", c % 4, biases(c));
    }

    // flush what remains in the buffer
    computeCode.printf("if (gl_GlobalInvocationID.x < %du && gl_GlobalInvocationID.y < %du) store(dpos(%d), actf(buf));",
        outputSize[0], outputSize[1], slice[1] / 4 - 1);
}


void Convolution2D::renderPointwise(const GraphicPipeline& gpu, StringBuilder& computeCode, StringBuilder& dataCode) {
    const int Z = ceili(kernelSize[2], 4);

    // define local group size
    localGroupSize[0] = std::min(2, inputSize[0]);
    localGroupSize[1] = std::min(2, inputSize[1]);
        //"2" is experimental; theoretical optimum is (int)std::sqrt((float)MAX_WORKGROUP_SIZE / localGroupSize[2]);

    do {
        localGroupSize[2] = std::mmin(Z,
            gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_TOTAL) / (localGroupSize[0] * localGroupSize[1]),
            gpu.getLimit(GraphicPipeline::Limit::LOCAL_GROUPS_Z));
        if (localGroupSize[2] > 0)
            break;

        localGroupSize[0] /= 2;
        localGroupSize[1] /= 2;
    } while (true);

    do {
        const int requiredSharedMem = Storage::getEntrySize(inputType) * localGroupSize[0] * localGroupSize[1] * Z;
        if (requiredSharedMem <= gpu.getLimit(GraphicPipeline::Limit::SHARED_MEM))
            break;
        localGroupSize[0] = localGroupSize[1] /= 2;
    } while (true);

    // declare shared memory
    dataCode.printf("shared highp %s sharedInput[gl_WorkGroupSize.x][gl_WorkGroupSize.y][%d];", Storage::getTypeName(inputType).c_str(), Z);

    // compute
    computeCode.printf<768>(STRINGIFY(
        for (uint z = gl_LocalInvocationID.z; z < %du; z += gl_WorkGroupSize.z) {
            sharedInput[gl_LocalInvocationID.x][gl_LocalInvocationID.y][z] = loadPacked(ivec3(P0, z));
        }
        memoryBarrierShared();
        barrier();
        highp vec4 acc = b[gl_GlobalInvocationID.z];
        for (uint z = 0u; z < %du; z++) {
            acc += mat4(
                weightLoad(gl_GlobalInvocationID.z, 4u*z+0u),
                weightLoad(gl_GlobalInvocationID.z, 4u*z+1u),
                weightLoad(gl_GlobalInvocationID.z, 4u*z+2u),
                weightLoad(gl_GlobalInvocationID.z, 4u*z+3u)) * unpack(sharedInput[gl_LocalInvocationID.x][gl_LocalInvocationID.y][z]);
        }
    ),
        Z, Z);

    // store
    if (inputSize[0] % localGroupSize[0] != 0 || inputSize[1] % localGroupSize[1] != 0) {
        Size outputSize = getOutputSize();
        computeCode.printf("if (gl_GlobalInvocationID.x < %du && gl_GlobalInvocationID.y < %du)", outputSize.getWidth(), outputSize.getHeight());
    }
    if (ceili(outChannelsNum, localGroupSize[2]) * localGroupSize[2] > outChannelsNum) {
        computeCode.printf("if (gl_GlobalInvocationID.z < %du)", ceili(outChannelsNum, 4));
    }
    computeCode.printf("store(ivec3(gl_GlobalInvocationID), actf(acc));");
}


std::vector<std::string> Convolution2D::generateCode(GraphicPipeline& gpu, ChunkFile& data) {
    std::string commonCode;
    StringBuilder commonCodeBuild(commonCode);

    localGroupSize = Size::ONES;

    // determine number of programs
    const int numSlices = getNumberOfSlices();
    std::vector<std::string> code(numSlices);

    BEATMUP_DEBUG_I("-- %s : %d %d\n", getName().c_str(), kernelSize.volume() * outChannelsNum, numSlices);

    typedef float datatype;
    const std::string
        chunkW(getName() + "/W"),
        chunkB(getName() + "/b");

    Array<datatype, 4> kernels(outChanNumRoundUp4, kernelSize[2], kernelSize[0], kernelSize[1]);
    Array<datatype, 1> biases(outChanNumRoundUp4);

    // check data
    if (!data.chunkExists(chunkW))
        throw InconsistentModelData(this, "weights chunk not found");
    if (!data.chunkExists(chunkB))
        throw InconsistentModelData(this, "biases chunk not found");
    if (data.chunkSize(chunkW) != kernels.size() * sizeof(datatype))
        throw InconsistentModelData(this, "weights size mismatch", kernels.size(), data.chunkSize(chunkW));
    if (data.chunkSize(chunkB) != biases.size() * sizeof(datatype))
        throw InconsistentModelData(this, "biases size mismatch", biases.size(), data.chunkSize(chunkB));

    // fetch data
    data.fetch(chunkB, biases.data(), biases.size() * sizeof(datatype));
    data.fetch(chunkW, kernels.data(), kernels.size() * sizeof(datatype));

    // declare inputs & outputs
    Storage::CodeGenerator accessCode;
    accessCode
        ("inputTensor",  getInputType(),  getInputSize(),  0, "readonly",  Storage::CodeGenerator::Access::LOADING)
        ("outputTensor", getOutputType(), getOutputSize(), 1, "writeonly", Storage::CodeGenerator::Access::STORING);
    commonCodeBuild(accessCode.get());

    // init GPU buffers
    if (mode == ComputingMode::BUFFERS) {
        // declare
        commonCodeBuild.printf<512>(STRINGIFY(
            layout(binding = %d, std430) buffer Weights{ highp vec4 w[]; };
            layout(binding = %d, std430) buffer Biases { highp vec4 b[]; };
            highp vec4 weightLoad(uint c, uint z) {
                return w[ z * %du + c ];
            }
            highp vec4 weightLoad(uvec4 p) {
                return w[ ((p.y * %du + p.x) * %du + p.z) * %du + p.w ];
            }
        ),
            GLSL_BINDING_WEIGHTS, GLSL_BINDING_BIASES,
            ceili(outChannelsNum, 4),
            kernelSize[0], kernelSize[2], ceili(outChannelsNum, 4));

        // allocate
        this->weights.allocate(gpu, kernelSize.volume() * outChanNumRoundUp4 * sizeof(float), kernels.data());
        this->biases.allocate(gpu, outChanNumRoundUp4 * sizeof(float), biases.data());
    }

    for (int sliceIdx = 0; sliceIdx < numSlices; ++sliceIdx) {
        std::string dataCode(commonCode), computeCode;
        StringBuilder dataCodeBuild(dataCode);
        StringBuilder computeCodeBuild(computeCode);

        // pick a slice of output channels
        const int
            slice[2] = {
                outChannelsNum * sliceIdx / numSlices / 4 * 4,
                outChannelsNum * (sliceIdx + 1) / numSlices / 4 * 4
            },
            sliceSize = slice[1] - slice[0];

        if (depthwise) {
            RuntimeError::check(kernelSize[2] == 1, "Depthwise kernels must be of depth 1");

            if (mode == ComputingMode::ARRAYS) {
                renderDepthwiseArrayBased(gpu, sliceIdx, computeCodeBuild, dataCodeBuild, kernels, biases);
            }
            else if (mode == ComputingMode::BUFFERS) {
                renderDepthwise(gpu, sliceIdx, computeCodeBuild, dataCodeBuild);
            }
            else {
                renderDepthwiseInline(gpu, sliceIdx, computeCodeBuild, dataCodeBuild, kernels, biases);
            }
        }

        else {
            RuntimeError::check(kernelSize[2] == inputSize.getDepth(), "Input and kernel depth mismatch");

            if (kernelSize[0] == 1 && kernelSize[1] == 1) {
                renderPointwise(gpu, computeCodeBuild, dataCodeBuild);
            }
            else if (mode == ComputingMode::ARRAYS) {
                renderArrayBased(gpu, sliceIdx, computeCodeBuild, dataCodeBuild, kernels, biases);
            }
            else {
                renderInline(gpu, sliceIdx, computeCodeBuild, dataCodeBuild, kernels, biases);
            }
        }

        // make source code
        code[sliceIdx] = BEATMUP_SHADER_CODE_V(
            <<LYT>>
            <<DATA>>
            ivec3 dpos(int channel) {
                return ivec3(gl_GlobalInvocationID.xy, channel);
            }
            highp vec4 src(ivec2 pos, int dx, int dy, int z) {
                return load(ivec3(pos + ivec2(dx, dy), z));
            }
            highp vec4 actf(vec4 _) { return 'ACTF'; }
            void main() {
                mediump ivec2 P0 = ivec2(gl_GlobalInvocationID.xy) * ivec2(<<STR>>) + ivec2(<<OFF>>);
                <<CC>>
                memoryBarrier();
            }
        );

        StringBuilder builder(code[sliceIdx]);
        builder
            .replace("<<LYT>>", "layout(local_size_x = " +
                std::to_string(localGroupSize[0]) + ", local_size_y = " +
                std::to_string(localGroupSize[1]) + ", local_size_z = " +
                std::to_string(localGroupSize[2]) + ") in;")
            .replace("<<DATA>>", dataCode)
            .replace("<<CC>>", computeCode)
            .replace("<<STR>>", stride.toString<2>())
            .replace("<<OFF>>", inputSize.getOrigin(kernelSize, stride, padding).toString<2>());

        switch (actFunc) {
        case ActivationFunc::BRELU6:
            builder.replace("'ACTF'", "min(max(vec4(0, 0, 0, 0), _), vec4(6, 6, 6, 6))");
            break;
        case ActivationFunc::IDENTITY:
            builder.replace("'ACTF'", "_");
            break;
        }
    }

    return code;
}


void Convolution2D::perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int workerIdx) const {
    Feedforward2DGPUOperation::perform(gpu, program, workerIdx);

    const int
        numSlices = getNumberOfSlices(),
        slice[2] = {
            outChannelsNum * workerIdx / numSlices / 4 * 4,
            outChannelsNum * (workerIdx + 1) / numSlices / 4 * 4
        };

    Size size;
    if (mode == ComputingMode::INLINE) {
        size = Size(output->getWidth(), output->getHeight(), 1);
    }
    else if (mode == ComputingMode::ARRAYS) {
        size = Size(output->getWidth(), output->getHeight(), ceili(slice[1] - slice[0], 4));
    }

    else if (mode == ComputingMode::BUFFERS) {
        weights.bind(gpu, GLSL_BINDING_WEIGHTS);
        biases.bind(gpu, GLSL_BINDING_BIASES);
        size = Size(output->getWidth(), output->getHeight(), outChannelsNum / 4);
    }

    program.dispatch(gpu, ceili(size[0], localGroupSize[0]), ceili(size[1], localGroupSize[1]), ceili(size[2], localGroupSize[2]));
}

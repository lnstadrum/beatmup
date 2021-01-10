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

/*
    Bunch of unit tests
*/

#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include "shading/shader_applicator.h"
#include "bitmap/internal_bitmap.h"
#include "context.h"
#include "gpu/float16.h"
#include "gpu/linear_mapping.h"
#include "gpu/swapper.h"
#include "nnets/deserialized_model.h"
#include "nnets/inference_task.h"
#include "shading/image_shader.h"
#include "utils/bitmap_from_chunk.h"
#include "utils/string_utils.h"
#include "debug.h"

using namespace Beatmup;

static bool verbose = true;


class GpuTestTask : public GpuTask {
protected:
    Context context;
public:
    GpuTestTask() {}

    void operator()() {
        context.performTask(*this);
    }

    static std::vector<float> makeRandomQuantizedVector(const int length, const int seed = 123) {
        std::vector<float> data(length);
        std::uniform_int_distribution<int> ran(0, 255);
        std::default_random_engine dom(seed);
        for (auto &_ : data)
            _ = ran(dom) / 255.0f;
        return data;
    }

    static std::vector<float> makeRandomVector(const int length, const float min, const float max, const int seed = 123) {
        std::vector<float> data(length);
        std::uniform_real_distribution<float> ran(min, max);
        std::default_random_engine dom(seed);
        for (auto &_ : data)
            _ = ran(dom);
        return data;
    }
};


/**
    Simple inputless image shader filling color channels according to a specific pattern
*/
class ImageShaderTest {
    Context context;
public:
    ImageShaderTest() {}
    void operator()() {
        ImageShader shader(context);
        shader.setSourceCode(BEATMUP_SHADER_CODE(
                void main() {
                    gl_FragColor = vec4(0, 0.25, 0.501, 1.0);
                }
        ));
        InternalBitmap bitmap(context, PixelFormat::QuadByte, 64, 64);
        ShaderApplicator applicator;
        applicator.setShader(&shader);
        applicator.setOutputBitmap(&bitmap);
        context.performTask(applicator);
        Swapper::pullPixels(bitmap);
        {
            AbstractBitmap::ReadLock lock(bitmap);
            const pixbyte *ptr = (pixbyte *) bitmap.getData(0, 0);
            static const int EXPECTED_PATTERN[] = {0, 64, 128, 255};
            if (ptr[0] != EXPECTED_PATTERN[0] || ptr[1] != EXPECTED_PATTERN[1] || ptr[2] != EXPECTED_PATTERN[2] || ptr[3] != EXPECTED_PATTERN[3]) {
                std::stringstream ss;
                ss << "ImageShader test failed. Expected pattern is not met, instead we got: "
                    << (int) ptr[0] << " " << (int) ptr[1] << " " << (int) ptr[2] << " " << (int) ptr[3] << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
    }
};


/**
    Encoding/decoding of Float16.
    FIXME: make this passing on Pi
*/
class Float16Test {
    Context context;
    ShaderApplicator applicator;
    ImageShader shader;
    InternalBitmap input, output;

    bool runEncodingTest(std::vector<float>& values) {
        // encode-decode the input values; Float16 compression is idempotent, this ensures zero error
        for (auto& value : values)
            value = (float)GL::Float16(value);

        // send the test vector to the shader
        shader.setFloatArray("values", values);

        // run shader application
        context.performTask(applicator);

        // get pixels containing processed values
        Swapper::pullPixels(output);

        // compare values
        AbstractBitmap::ReadLock lock(output);
        const pixbyte *ptr = (pixbyte *) output.getData(0, 0);
        float error = 0;
        for (size_t i = 0; i < values.size(); ++i, ptr += 4) {
            GL::Float16 testVal(ptr[0], ptr[1]);
            error = std::max(error, std::abs(values[i] - (float)testVal));
        }

        return error <= std::numeric_limits<float>::epsilon();
    }


    bool runDecodingTest(std::vector<float>& values) {
        // fill the input bitmap with encoded values
        {
            AbstractBitmap::WriteLock<ProcessingTarget::CPU> lock(input);
            pixbyte *ptr = (pixbyte *) input.getData(0, 0);
            for (size_t i = 0; i < values.size(); ++i, ptr += 4) {
                GL::Float16 value(values[i]);
                ptr[0] = value.getFrac();
                ptr[1] = value.getExp();
                values[i] = value;
            }
        }

        // send the test data to the shader
        shader.setFloatArray("refValues", values);
        applicator.addSampler(&input, "encValues");

        // run shader application
        context.performTask(applicator);

        // get pixels containing processed values
        Swapper::pullPixels(output);

        // decode differences
        AbstractBitmap::ReadLock lock(output);
        const pixbyte *ptr = (pixbyte *) output.getData(0, 0);
        float error = 0;
        for (size_t i = 0; i < values.size(); ++i, ptr += 4) {
            GL::Float16 diff(ptr[0], ptr[1]);
            error = std::max(error, (float)diff);
        }

        return error <= std::numeric_limits<float>::epsilon();
    }

public:
    Float16Test():
        shader(context),
        input(context, PixelFormat::QuadByte, 128, 1),
        output(context, PixelFormat::QuadByte, 128, 1)
    {
        applicator.setShader(&shader);
        applicator.setOutputBitmap(&output);
    }

    void operator()() {
        // test conversion in [-128, 127] range first
        float error = 0, intError = 0;
        for (int i = -12800; i <= 12700; ++i) {
            float x = i * 0.01f;
            GL::Float16 y(x);
            GL::Float16 y2(y);
            if (y2.getFrac() != y.getFrac() || y2.getExp() != y.getExp())
                throw std::runtime_error("Float16 idempotence test fail");
            error = std::max(error, std::abs(y-x));
            if (i % 100 == 0)
                intError = std::max(intError, std::abs(y-x));
        }
        if (error > 1)
            throw std::runtime_error("Float16 test error is too big: " + std::to_string(error));
        if (intError != 0)
            throw std::runtime_error("Float16 test integer error is too big: " + std::to_string(error));

        // test GLSL encoding
        {
            // shader receives a vector of values to encode, and writes the encoded result to output
            shader.setSourceCode(BEATMUP_SHADER_CODE(
                    uniform highp float values[128];
                )
                + GL::Float16::encodeGlsl() +
                R"(void main() {
                    lowp vec2 enc = encode(values[int(gl_FragCoord.x)]);
                    gl_FragColor = vec4(enc, 0.0, 1.0);
                })"
            );

            std::vector<float> values(input.getWidth());

            for (int i = -64; i < 64; ++i)
                values[i+64] = i;
            if (!runEncodingTest(values))
                throw std::runtime_error("Float16 GLSL encoding test integer error is too big");

            for (int i = -64; i < 64; ++i)
                values[i+64] = i * 0.0123f;
            if (!runEncodingTest(values))
                throw std::runtime_error("Float16 GLSL encoding test error is too big");

            for (int i = -64; i < 64; ++i)
                values[i+64] = i * 55;
            if (!runEncodingTest(values))
                throw std::runtime_error("Float16 GLSL encoding test error is too big");
        }

        shader.clear();

        // test GLSL decoding
        {
            // shader reads encoded values, decodes them and encodes the difference with the reference
            shader.setSourceCode(BEATMUP_SHADER_CODE(
                    uniform highp float refValues[128];
                    uniform sampler2D encValues;
                )
                + std::string(GL::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG) +
                GL::Float16::encodeGlsl() +
                GL::Float16::decodeGlsl() +
                R"(void main() {
                    highp float refValue = refValues[int(gl_FragCoord.x)];
                    highp float testValue = decode(texture2D(encValues, texCoord).xy);
                    lowp vec2 enc = encode(testValue - refValue);
                    gl_FragColor = vec4(enc, 0.0, 1.0);
                })"
            );

            std::vector<float> values(input.getWidth());
            float error;

            for (int i = -64; i < 64; ++i)
                values[i+64] = i;
            if (!runDecodingTest(values))
                throw std::runtime_error("Float16 GLSL decoding test integer error is too big");

            for (int i = -64; i < 64; ++i)
                values[i+64] = i * 0.0123f;
            if (!runDecodingTest(values))
                throw std::runtime_error("Float16 GLSL decoding test error is too big");

            for (int i = -64; i < 64; ++i)
                values[i+64] = i * 55;
            if (!runDecodingTest(values))
                throw std::runtime_error("Float16 GLSL decoding test error is too big");
        }
    }
};



/**
    Sending a random vector to GPU memory converting it to 16-bit fixed point representation, fetching back and comparing.
*/
class GpuFixedPointVectorTest : public GpuTestTask {
    bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
        static const int SIZE = 256;
        static const float ERROR_THRESHOLD = 1.1f / 512;

        // make random vector
        std::vector<float> vector = makeRandomVector(SIZE, -10, 10), swapped;

        // put to GPU memory and read back
        GL::Vector glVector(context, gpu, vector.size(), GL::Vector::Format::FIXED16, vector.data());
        glVector.fetch(gpu, swapped);

        // compare
        float err = 0;
        for (size_t i = 0; i < vector.size(); ++i)
            err = std::max(err, std::abs(swapped[i] - vector[i]));
        if (err > ERROR_THRESHOLD)
            throw RuntimeError("GPU fixed-point vector test fail. Max abs error: " + std::to_string(err));

        return true;
    }
};


/**
    Multiplying a random vector by identity matrix
*/
class IdentityMatrixMultiplicationTest : public GpuTestTask {
    const int size;
    const GL::Vector::Format format;

    bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
        static const float ERROR_THRESHOLD = 1.1f / 256;

        // make identity matrix
        std::vector<float> matrix(size * size);
        for (int y = 0, i = 0; y < size; ++y)
            for (int x = 0; x < size; ++x, ++i)
                matrix[i] = x == y ? 1.0f : 0.0f;

        // make random vector
        auto vector = makeRandomQuantizedVector(size);

        // set up GL stuff
        GL::Vector glVector(context, gpu, size, format, vector.data());
        GL::Vector glResult(context, gpu, size, format);

        // create mapping
        GL::LinearMapping mapping(context, format != GL::Vector::Format::FLOAT);
        mapping.setMatrix(gpu, size, size, matrix.data());

        // compute and get the result
        mapping(gpu, glResult, glVector);
        std::vector<float> result;
        glResult.fetch(gpu, result);

        // compare
        float err = 0;
        for (size_t i = 0; i < result.size(); ++i)
            err = std::max(err, std::abs(vector[i] - result[i]));
        if (err > ERROR_THRESHOLD)
            throw RuntimeError("Multiplying by identity matrix test fail. Max abs error: " + std::to_string(err));

        return true;
    }

public:
    IdentityMatrixMultiplicationTest(const int size, const GL::Vector::Format format) :
        size(size), format(format)
    {}
};


/**
    Applying linear mapping to a random vector
*/
class LinearMappingTest : public GpuTestTask {
    const int width, height;
    const bool force16bit;

    bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
        // make random matrix
        std::vector<float> matrix = makeRandomVector(width * height, -1, 1, 12345);

        // make random vector
        std::vector<float> vector = makeRandomQuantizedVector(width);

        // make random bias
        std::vector<float> bias = makeRandomVector(width * height, -1, 1, 123456);

        // set up GL stuff
        GL::Vector glVector(context, gpu, width, GL::Vector::Format::TEXTURE, vector.data());
        GL::Vector glResult(context, gpu, height, force16bit ? GL::Vector::Format::FIXED16 : GL::Vector::DEFAULT_FORMAT);

        // compute ground truth
        std::vector<float> gt(height);
        for (size_t j = 0, k = 0; j < height; ++j) {
            gt[j] = bias[j];
            for (size_t i = 0; i < width; ++i, ++k)
                gt[j] += matrix[k] * vector[i];
            if (glResult.getDataFormat() == GL::Vector::Format::TEXTURE)
                gt[j] = std::min(std::max(0.0f, gt[j]), 1.0f);
        }

        // create mapping
        GL::LinearMapping mapping(context, force16bit);
        mapping.setMatrix(gpu, width, height, matrix.data());
        mapping.setBias(gpu, height, bias.data());
        mapping(gpu, glResult, glVector);

        std::vector<float> result;
        glResult.fetch(gpu, result);

        float err = 0;
        for (size_t i = 0; i < result.size(); ++i)
            err = std::max(err, std::abs(gt[i] - result[i]));

        std::cout << "  Error: " << err << std::endl;
            //fixme: need a threshold here

        return true;
    }

public:
    LinearMappingTest(const int width, const int height, const bool force16bit) :
        width(width), height(height), force16bit(force16bit)
    {}
};


class StoragePushingPullingTest : public GpuTestTask {
    Context ctx;

    bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
        const NNets::Size size(256, 256, 12);
        NNets::Storage storage(ctx, gpu, size, 0);

        // generate data
        auto data = makeRandomQuantizedVector(size.volume());

        // push and pull
        storage.push(gpu, data.data(), data.size());
        gpu.flush();    // for fun
        storage.pull(gpu);

        // check
        for (int j = 0, k = 0; j < size[2]; j+=4)
            for (int i = 0; i < size[0] * size[1]; ++i)
                for (int jj = j; jj < j+4; ++jj, ++k) {
                    if (storage.getMemory()[k] / 255.0f != data[i * size[2] + jj]) {
                        std::cout << storage.getMemory().at<uint8_t>(k) / 255.0f << "  " << data[i * size[2] + jj] << std::endl;
                        throw std::runtime_error("Storage data corrupted");
                    }
                }

        return true;
    }
};


int main() {
    try {
        std::cout << "Basic shading test..." << std::endl;
        ImageShaderTest()();

        std::cout << "Linear mapping test..." << std::endl;
        GpuFixedPointVectorTest()();

#ifndef BEATMUP_OPENGLVERSION_GLES20
        IdentityMatrixMultiplicationTest(1024, GL::Vector::Format::FLOAT)();
#endif
        IdentityMatrixMultiplicationTest(1024, GL::Vector::Format::FIXED16)();
        IdentityMatrixMultiplicationTest(1024, GL::Vector::Format::TEXTURE)();

#ifndef BEATMUP_OPENGLVERSION_GLES20
        LinearMappingTest(1024, 1000, true)();
#endif
        LinearMappingTest(1024, 1000, false)();

        std::cout << "Storage push and pull test..." << std::endl;
        StoragePushingPullingTest()();

        // replaying
        static const char* TESTS_FILE = "tests.chunks";
        if (ChunkFile::readable(TESTS_FILE)) {
            Context context;
            ChunkFile testsFile(TESTS_FILE, true);

            int testNumber = 1;
            std::string prefix;

            while (testsFile.chunkExists(prefix = "test" + std::to_string(testNumber))) {
                // read test data
                testsFile.open();

                // get and print title
                const std::string title = testsFile.read<std::string>(prefix);
                std::cout << "#" << testNumber << ": " << title << std::endl;

                // get model
                const std::string code = testsFile.read<std::string>(prefix + ":model");

                // get test input
                int inputShape[3];
                testsFile.fetch(prefix + ":input_shape", inputShape, sizeof(inputShape));
                InvalidArgument::check(inputShape[2] == 3, "3-channel test input bitmap expected");
                InternalBitmap input(context, PixelFormat::TripleByte, inputShape[1], inputShape[0]);
                BitmapFromChunk::load(input, testsFile, prefix + ":input");

                // get reference output and error threshold
                const auto groundTruth = testsFile.readVector<float>(prefix + ":gt");
                const float errorThreshold = testsFile.read<float>(prefix + ":threshold");
                testsFile.close();

                // build model
                std::istringstream stream(code);
                NNets::DeserializedModel model(context, Listing(stream));

                // init inference task
                NNets::InferenceTask inference(model, testsFile);

                // connect input
                inference.connect(input, model.getFirstOperation());

                // connect output if not softmax
                auto& head = model.getLastOperation();
                const bool isSoftmax = StringUtils::lowercase(head.getName()).find("softmax") != std::string::npos;
                if (!isSoftmax)
                    model.addOutput(head);

                // run inference
                try {
                    context.performTask(inference);
                }
                catch (const std::exception & ex) {
                    throw std::runtime_error(title + " failed:\n" +  code + "\n\n" + ex.what());
                }

                // get test output
                std::vector<float> output;
                if (isSoftmax)
                    output = static_cast<NNets::Softmax&>(head).getProbabilities();
                else {
                    output.resize(head.getOutputSize().volume());
                    size_t numSamples;
                    std::memcpy(output.data(), model.getOutputData(numSamples, head), output.size() * sizeof(float));
                }

                // get max absolute difference
                RuntimeError::check(output.size() == groundTruth.size(), "Output size does not match ground truth size");
                float maxDiff = 0;
                for (size_t i = 0; i < output.size(); ++i) {
                    float diff = std::abs(output[i] - groundTruth[i]);
                    maxDiff = std::max(maxDiff, diff);
                }

                // print error
                std::cout << "  Error: " << maxDiff << std::endl;
                BEATMUP_DEBUG_I("Test %d: %0.4f", testNumber, maxDiff);

                // compare with threshold
                if (maxDiff >= errorThreshold)
                    throw std::runtime_error(title + " failed, error " + std::to_string(maxDiff) +
                        " is greater than threshold " + std::to_string(errorThreshold));

                // clean up and go to next test
                context.getGpuRecycleBin()->emptyBin();
                ++testNumber;
            }
        }

    }
    catch (const std::exception & ex) {
        std::cout << ex.what() << std::endl << "Failed." << std::endl;
        return -1;
    }

    std::cout << "All OK." << std::endl;
    return 0;
}

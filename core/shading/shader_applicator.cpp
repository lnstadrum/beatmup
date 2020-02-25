#include "shader_applicator.h"
#include "../gpu/pipeline.h"
#include "../debug.h"

using namespace Beatmup;

bool ShaderApplicator::processOnGPU(GraphicPipeline &gpu, TaskThread &thread) {
    shader->prepare(gpu, mainInput, output, AffineMapping::IDENTITY);
    
    // binding other inputs
    size_t unit = mainInput ? 1 : 0;
    for (auto input : samplers) {
        gpu.bind(*input.second, unit, TextureParam::INTERP_LINEAR);
        shader->setInteger(input.first, (int)unit);
        unit++;
    }

    shader->process(gpu);
    return true;
}


void ShaderApplicator::beforeProcessing(ThreadIndex threadCount, GraphicPipeline *gpu) {
    NullTaskInput::check(output, "output bitmap");
    NullTaskInput::check(shader, "image shader");
    if (mainInput)
        mainInput->lockPixels(ProcessingTarget::GPU);
    for (auto _ : samplers)
        _.second->lockPixels(ProcessingTarget::GPU);
    output->lockPixels(ProcessingTarget::GPU);
}


void ShaderApplicator::afterProcessing(ThreadIndex threadCount, bool aborted) {
    if (mainInput)
        mainInput->unlockPixels();
    for (auto _ : samplers)
        _.second->unlockPixels();
    output->unlockPixels();
}


ShaderApplicator::ShaderApplicator():
    shader(nullptr), mainInput(nullptr), output(nullptr)
{}


void ShaderApplicator::addSampler(AbstractBitmap* bitmap, const std::string uniformName) {
    if (uniformName == ImageShader::INPUT_IMAGE_ID)
        mainInput = bitmap;
    else
        samplers[uniformName] = bitmap;
}


bool ShaderApplicator::removeSampler(const std::string uniformName) {
    if (uniformName == ImageShader::INPUT_IMAGE_ID)
        if (mainInput) {
            mainInput = nullptr;
            return true;
        }
        else
            return false;

    auto sampler = samplers.find(uniformName);
    if (sampler == samplers.end()) {
        return false;
    }
    else {
        samplers.erase(sampler);
        return true;
    }
}


void ShaderApplicator::clearSamplers() {
    mainInput = nullptr;
    samplers.clear();
}


void ShaderApplicator::setOutputBitmap(AbstractBitmap *bitmap) {
    output = bitmap;
}


void ShaderApplicator::setShader(ImageShader *shader) {
    this->shader = shader;
}
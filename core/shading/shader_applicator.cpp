/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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

#include "shader_applicator.h"
#include "../gpu/pipeline.h"
#include "../debug.h"

using namespace Beatmup;

bool ShaderApplicator::processOnGPU(GraphicPipeline &gpu, TaskThread &thread) {
    // distribute texture units over samplers
    {
        int unit = mainInput ? 1 : 0;
        for (auto input : samplers)
            if (input.first != ImageShader::INPUT_IMAGE_ID) {
                RuntimeError::check(input.second->getTextureFormat() != GL::TextureHandler::TextureFormat::OES_Ext,
                    "OES_EXT samplers are only supported when bound to '" + ImageShader::INPUT_IMAGE_ID + "' sampler variable of type " + ImageShader::INPUT_IMAGE_DECL_TYPE);
                shader->setInteger(input.first, unit);
                unit++;
            }
    }

    // prepare shader
    shader->prepare(gpu, mainInput, output);

    // bind textures
    {
        size_t unit = mainInput ? 1 : 0;
        for (auto input : samplers)
            if (input.first != ImageShader::INPUT_IMAGE_ID) {
                gpu.bind(*input.second, unit, TextureParam::INTERP_LINEAR);
                unit++;
            }
    }

    // process
    shader->process(gpu);
    return true;
}


void ShaderApplicator::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline *gpu) {
    NullTaskInput::check(output, "output bitmap");
    NullTaskInput::check(shader, "image shader");
    if (mainInput)
        readLock(gpu, mainInput, ProcessingTarget::GPU);
    for (auto _ : samplers)
        readLock(gpu, _.second, ProcessingTarget::GPU);
    writeLock(gpu, output, ProcessingTarget::GPU);
}


void ShaderApplicator::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    if (mainInput)
        unlock(mainInput);
    for (auto _ : samplers)
        unlock(_.second);
    unlock(output);
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
    if (uniformName == ImageShader::INPUT_IMAGE_ID) {
        if (mainInput) {
            mainInput = nullptr;
            return true;
        }
        else
            return false;
    }

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

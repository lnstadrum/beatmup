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

#include "image_sampler.h"
#include "../gpu/bgl.h"

using namespace Beatmup;
using namespace NNets;


static const char *UNIFORM_INPUT = "img";

ImageSampler::ImageSampler(
    const std::string& name,
    const IntPoint& size,
    bool centerCrop,
    bool linearInterp
):
    AbstractOperation(name),
    size(size),
    input(nullptr), output(nullptr), program(nullptr),
    linearInterpolation(linearInterp), centerCrop(centerCrop), rotation(0)
{}


void ImageSampler::getOutput(GL::TextureHandler*& texture, int index) {
    InvalidArgument::check(index == 0, "Invalid output index of ImageSampler operation: " + std::to_string(index));
    texture = this->output;
}


void ImageSampler::setInput(GL::TextureHandler& texture, int inputIndex) {
    InvalidArgument::check(inputIndex == 0, "Invalid input index of ImageSampler operation: " + std::to_string(inputIndex));
    this->input = &texture;
}


void ImageSampler::setOutput(GL::TextureHandler& texture, int outputIndex) {
    InvalidArgument::check(outputIndex == 0, "Invalid output index of ImageSampler operation: " + std::to_string(outputIndex));
    InvalidArgument::check(texture.getWidth() == size.x && texture.getHeight() == size.y,
        "ImageSampler output size mismatch");
    this->output = &texture;
}


bool ImageSampler::initDeserializer() {
    static class ImageSamplerDeserializer : public AbstractOperation::Deserializer {
    public:
        ImageSamplerDeserializer() : Deserializer("image_sampler") {}
        AbstractOperation* deserialize(Context& context, const Listing::Block& block) {
            /** \page NNetsOpsSerialization
                \section ImageSampler
                \code{yaml}
                - _name: arbitrary operation name
                  _type: image_sampler  # fixed string
                  output_width: 224     # resulting image width in pixels
                  output_height: 224    # resulting image width in pixels
                  center_crop: true     # center crop enabled, "true" or "false" (defaults to "true")
                  linear_interp: true   # linear interpolation of input pixels enabled, "true" or "false" (defaults to "true")
                \endcode
            */
            return new ImageSampler(
                block["_name"],
                IntPoint(block.get<int>("output_width"), block.get<int>("output_height")),
                block.get<bool>("center_crop", true),
                block.get<bool>("linear_interp", true)
            );
        }
    } john;

    return true;
}


std::map<std::string, std::string> ImageSampler::serialize() const {
    return {
        { "_name",          getName() },
        { "_type",          "image_sampler" },
        { "output_width",   std::to_string(size.x) },
        { "output_height",  std::to_string(size.y) },
        { "linear_interp",  linearInterpolation ? "true" : "false" },
        { "center_crop",    centerCrop ? "true" : "false" }
    };
}


void ImageSampler::disconnect() {
    input = output = nullptr;
}



unsigned long ImageSampler::countTexelFetches() const {
    return size.x * size.y;
}


void ImageSampler::prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank) {
    RuntimeError::check(input, "Input is not provided to a ImageSampler operation.");

    // delete the old program
    if (program)
        bank.release(gpu, program);

    // begin the shader code
    String code(GL::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG);

    // declare the sampler
    code.printf("uniform %s %s;", GL::FragmentShader::DIALECT_SAMPLER_DECL_TYPE, UNIFORM_INPUT);

    // add main code
    code.printf(R"glsl(
        void main() {
            gl_FragColor = %s(%s, %s);
        }
    )glsl", GL::FragmentShader::DIALECT_TEXTURE_SAMPLING_FUNC, UNIFORM_INPUT, GL::RenderingPrograms::TEXTURE_COORDINATES_ID);

    // init program
    bool enableExtTexExt = input->getTextureFormat() == GL::TextureHandler::TextureFormat::OES_Ext;
    program = bank(gpu, code, enableExtTexExt);
}


void ImageSampler::execute(TaskThread& thread, GraphicPipeline& gpu) {
    if (!program)
        throw NotReady(this);
    RuntimeError::check(input, "Input is not provided to a ImageSampler operation.");
    RuntimeError::check(output, "Output is not provided to a ImageSampler operation.");

    // enable program
    program->enable(gpu);

    // bind output
    gpu.bindOutput(*output);

    // bind input
    gpu.bind(*input, 0, linearInterpolation ? TextureParam::INTERP_LINEAR : TextureParam::INTERP_NEAREST);

    // setup texture coordinates
    const IntPoint inputSize(input->getWidth(), input->getHeight());
    Rectangle texCoords;
    if (centerCrop) {
        float hMargin = 0, vMargin = 0;
        if (input->getWidth() * output->getHeight() > input->getHeight() * output->getWidth()) {
            // input is cut vertically
            hMargin = 0.5f * (input->getWidth() - (float)input->getHeight() * output->getWidth() / output->getHeight());
        }
        else {
            // input is cut horizontally
            vMargin = 0.5f * (input->getHeight() - (float)input->getWidth() * output->getHeight() / output->getWidth());
        }
        texCoords = gpu.getTextureCoordinates(Rectangle(hMargin, vMargin, input->getWidth() - 1 - hMargin, input->getHeight() - 1 - vMargin), inputSize, size);
    }
    else {
        texCoords = gpu.getTextureCoordinates(Rectangle(0, 0, input->getWidth() - 1, input->getHeight() - 1), inputSize, size);
    }

    // apply rotation
    if (rotation % 4 != 0) {
        Point topLeft(texCoords.a), topRight(texCoords.b.x, texCoords.a.y), bottomLeft(texCoords.a.x, texCoords.b.y), bottomRight(texCoords.b), tmp;
        switch (rotation % 4) {
            case 1:
                tmp = bottomLeft;
                bottomLeft = bottomRight;
                bottomRight = topRight;
                topRight = topLeft;
                topLeft = tmp;
                break;
            case 2:
                tmp = topLeft;
                topLeft = bottomRight;
                bottomRight = tmp;
                tmp = topRight;
                topRight = bottomLeft;
                bottomLeft = tmp;
                break;
            case 3:
                tmp = topLeft;
                topLeft = topRight;
                topRight = bottomRight;
                bottomRight = bottomLeft;
                bottomLeft = tmp;
                break;
        }
        gpu.setTextureCoordinates(topLeft, topRight, bottomLeft, bottomRight);
    }
    else {
        gpu.setTextureCoordinates(texCoords);
    }

    // blend
    program->blend();
}

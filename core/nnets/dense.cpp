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

#include "dense.h"
#include "deserialized_model.h"

using namespace Beatmup;
using namespace NNets;


const char *Dense::MATRIX_CHUNK_SUFFIX = "/w";
const char *Dense::BIAS_CHUNK_SUFFIX = "/b";


Dense::Dense(Context& context, const std::string& name, const int numOutputDims, bool useBias):
    AbstractOperation(name),
    LinearMapping(context, false),
    numOutputDims(numOutputDims), useBias(useBias), inputVector(nullptr), outputVector(nullptr)
{
    OutOfRange::checkMin(numOutputDims, 1, "Positive number of output dimensions expected, %d got");
    Storage::checkChannelNumber(numOutputDims);
}


void Dense::getOutput(GL::Vector* &vector, int index) {
    OutOfRange::check(index, 0, 1, "Output index out of range: %d");
    vector = this->outputVector;
}


void Dense::setInput(Storage::View&& view, int index) {
    OutOfRange::check(index, 0, 1, "Input index out of range: %d");
    if (view) {
        RuntimeError::check(view.getStorage().getPadding() == 0, "Storages with padding are not supported");
        const auto& storage = view.getStorage();
        RuntimeError::check(view.getNumberOfTextures() == 1 && storage.getTextureWidth() == 1, "Input size mismatch: a column-like view is expected");
    }
    inputStorage = std::move(view);
    inputVector = nullptr;
}


void Dense::setInput(GL::Vector& vector, int index) {
    OutOfRange::check(index, 0, 1, "Input index out of range: %d");
    inputStorage = Storage::View();
    inputVector = &vector;
}


void Dense::setOutput(GL::Vector& vector, int index) {
    outputVector = &vector;
}


GL::Vector::Format Dense::getOutputVectorFormat() const {
#ifdef BEATMUP_OPENGLVERSION_GLES20
    return GL::Vector::Format::FIXED16;
#else
    return GL::Vector::Format::FLOAT;
#endif
}


std::map<std::string, std::string> Dense::serialize() const {
    return {
        { "_name",          getName() },
        { "_type",          "dense" },
        { "output_dims",    std::to_string(numOutputDims) },
        { "use_bias",       useBias ? "true" : "false" },
    };
}


bool Dense::initDeserializer() {
    static class DenseDeserializer : public AbstractOperation::Deserializer {
    public:
        DenseDeserializer() : Deserializer("dense") {}
        AbstractOperation* deserialize(Context& context, const Listing::Block& block) {
            /** \page NNetsOpsSerialization
                \section Dense
                \code{yaml}
                - _name: arbitrary operation name
                  _type: dense      # fixed string
                  use_bias: true    # bias addition, "true" or "false"
                \endcode
            */
            return new Dense(context,
                block["_name"],
                block.get<int>("output_dims"),
                block.get<bool>("use_bias")
            );
        }
    } john;

    return true;
}


void Dense::disconnect() {
    inputVector = nullptr;
    inputStorage = Storage::View();
    outputVector = nullptr;
}


void Dense::prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank) {
    RuntimeError::check(inputVector || inputStorage, "Input is not provided to Dense operation " + getName());
    RuntimeError::check(outputVector, "Output is not provided to Dense operation " + getName());

    const int numInputDims = inputVector ? inputVector->getSize() : inputStorage.getSize().volume();

    // set matrix
    const Chunk matrix(data, getName() + MATRIX_CHUNK_SUFFIX);
    if (matrix.size() != numInputDims * numOutputDims * sizeof(float))
        throw InconsistentModelData(this, "Matrix size mismatch");
    LinearMapping::setMatrix(gpu, numInputDims, numOutputDims, matrix.ptr<float>());

    // set bias
    if (useBias) {
        const Chunk bias(data, getName() + BIAS_CHUNK_SUFFIX);
        if (bias.size() != numOutputDims * sizeof(float))
            throw InconsistentModelData(this, "Bias size mismatch");
        LinearMapping::setBias(gpu, numOutputDims, bias.ptr<float>());
    }

    // prepare
    if (inputVector) {
        LinearMapping::prepare(gpu, *outputVector, *inputVector, &bank);
    }
    else {
        Storage::TextureHandler inputHandle(inputStorage, 0);
        LinearMapping::prepare(gpu, *outputVector, inputHandle, &bank);
    }
}


void Dense::execute(TaskThread& thread, GraphicPipeline& gpu) {
    if (inputVector) {
        LinearMapping::process(gpu, *outputVector, *inputVector);
    }
    else {
        Storage::TextureHandler inputHandle(inputStorage, 0);
        LinearMapping::process(gpu, *outputVector, inputHandle);
    }
}


void Dense::getSampledChannels(int index, int& min, int& max) const {
    min = max = (index == 0 ? 4 : 0);
}
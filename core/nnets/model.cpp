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

#include "model.h"
#include "../utils/bitset.h"
#include <sstream>

using namespace Beatmup;
using namespace NNets;


Model::Model(Context& context, std::initializer_list<AbstractOperation*> ops):
    ProgramBank(context),
    profiler(nullptr), ready(false),
    ops(ops.begin(), ops.end())
{
    // establish feedforward connections
    for (size_t i = 1; i < this->ops.size(); ++i)
        addConnection(*this->ops[i - 1], *this->ops[i]);
}


Model::Model(Context& context): Model(context, {}) {}

Model::~Model() {
    for (auto op : ops)
        op->disconnect();
    freeMemory();
}


void Model::append(AbstractOperation* newOp, bool connect) {
    for (auto op : ops) {
        if (op == newOp)
            throw RuntimeError("Cannot add operation " + newOp->getName() + " to the model: already added");
        else
            if (op->getName() == newOp->getName())
                throw RuntimeError("Cannot add operation " + newOp->getName() + " to the model: an operation with the same exists in the model");
    }
    ops.push_back(newOp);
    if (connect)
        addConnection(*ops[ops.size() - 2], *ops.back(), 0, 0, 0);
    ready = false;
}


void Model::append(std::initializer_list<AbstractOperation*> newOps, bool connect) { 
    for (auto op : newOps)
        append(op, connect);
}


void Model::addOperation(const std::string& opName, AbstractOperation* newOp) {
    auto it = std::find_if(ops.begin(), ops.end(), [&opName](AbstractOperation* op){ return op->getName() == opName; });
    if (it == ops.end())
        throw InvalidArgument("Cannot find operation " + opName);
    ops.insert(it, newOp);
}


void Model::addOperation(const AbstractOperation& op, AbstractOperation* newOp) {
    auto it = std::find(ops.begin(), ops.end(), &op);
    if (it == ops.end())
        throw InvalidArgument("Operation " + op.getName() + " is not in the model");
    ops.insert(it, newOp);
}


void Model::addConnection(const std::string& sourceOpName, const std::string& destOpName, int output, int input, int shuffle) {
    auto& source = getOperation(sourceOpName);
    auto& dest = getOperation(destOpName);
    addConnection(source, dest, output, input, shuffle);
}


void Model::addConnection(AbstractOperation& source, AbstractOperation& dest, int output, int input, int shuffle) {
    RuntimeError::check(0 <= output && output < source.getOutputCount(),
        "Operation " + source.getName() + " does not have output #" + std::to_string(output));
    RuntimeError::check(0 <= input && input < dest.getInputCount(),
        "Operation " + dest.getName() + " does not have input #" + std::to_string(input));
    connections.emplace(&source, Connection{ &dest, output, input, shuffle });
    ready = false;
}


void Model::addOutput(const std::string& opName, int output) {
    auto op = (*this)[opName];
    auto outputs = userOutputs.equal_range(op);
    for (auto i = outputs.first; i != outputs.second; ++i)
        if (i->second.index == output)
            // already added
            return;
    userOutputs.emplace(op, UserOutput{ output });
    ready = false;
}


void Model::addOutput(const AbstractOperation& operation, int output) {
    RuntimeError::check(isOperationInModel(operation), "Operation " + operation.getName() + " is not in the model");
    auto outputs = userOutputs.equal_range(&operation);
    for (auto i = outputs.first; i != outputs.second; ++i)
        if (i->second.index == output)
            // already added
            return;
    userOutputs.emplace(&operation, UserOutput{ output });
    ready = false;
}


const float* Model::getOutputData(size_t& numSamples, const std::string& operation, int output) const {
    return getOutputData(numSamples, *(*this)[operation], output);
}


const float* Model::getOutputData(size_t& numSamples, const AbstractOperation& operation, int output) const {
    auto outputs = userOutputs.equal_range(&operation);
    for (auto i = outputs.first; i != outputs.second; ++i)
        if (i->second.index == output) {
            numSamples = i->second.data.size();
            return i->second.data.data();
        }

    numSamples = 0;
    return nullptr;
}


void Model::prepare(GraphicPipeline& gpu,  ChunkCollection& data) {
    if (ready)
        return;
    freeMemory();

    std::map<Storage*, std::vector<AbstractOperation*>> refs;
        // Contains ops that use a specific storage as input, meaning that it cannot be reused elsewhere.
        // If no ops refer a storage, in can be recycled.

    // find input depth capping
    // If too many channels are sampled by an op having multiple inputs, its input storages will have reserved channels.
    const int sampledChannelsLimit = 4 * gpu.getLimit(GraphicPipeline::Limit::TEXTURE_IMAGE_UNITS);
    std::map<AbstractOperation*, int> sampledChannels;   // op => number of sampled channels
    for (auto conn : connections) {
        auto* op = conn.second.dest;
        // get the number of sampled channels
        int min, max;
        op->getSampledChannels(conn.second.input, min, max);
        // cap the maximum: a storage will not have more channels than the limit anyway
        max = std::min(max, sampledChannelsLimit);
        // add to input channels
        sampledChannels[op] += max;
    }

    // loop through connected ops
    data.open();
    preparingProgress.reset(ops.size());
    for (auto src : ops) {
        std::vector<Beatmup::Object*> outputs(src->getOutputCount(), nullptr);  // src output index => storage/vector bound to the output
        std::vector<int> paddings(src->getOutputCount(), 0);    // src output index => max padding over all connections
        Bitset connectedOutputs(src->getOutputCount(), false);

        // loop over connections to find max paddings per output
        auto connections = this->connections.equal_range(src);
        for (auto i = connections.first; i != connections.second; ++i) {
            const auto& conn = i->second;
            paddings[conn.output] = std::max(paddings[conn.output], conn.dest->getInputPadding(conn.input));
        }

        // loop over connections
        for (auto i = connections.first; i != connections.second; ++i) {
            const auto& conn = i->second;
            auto* dst = conn.dest;
            connectedOutputs.set(conn.output);

            if (outputs[conn.output])
                RuntimeError::check(src->acceptsStorageOutput(conn.output) ^ src->acceptsVectorOutput(conn.output) ^ src->acceptsTextureOutput(conn.output),
                    "Operation output accepting different types can only have a single connection");
                    // To avoid output type mismatch when connecting second time

            // if a regular Storage is accepted by both source and destination
            if (src->acceptsStorageOutput(conn.output) && dst->acceptsStorageInput(conn.input)) {
                const Size size = src->getOutputSize(conn.output);
                Storage* storage = nullptr;

                // check if the output storage is already allocated
                if (outputs[conn.output]) {
                    storage = static_cast<Storage*>(outputs[conn.output]);
                    refs[storage].push_back(dst);
                }

                else {
                    // decide on reserved depth (if capping)
                    int depthCapping = 0;
                    if (sampledChannels[dst] > sampledChannelsLimit) {
                        // the op exceeds the limit
                        int min, max;
                        dst->getSampledChannels(conn.input, min, max);
                        const int cappingMargin = std::min(sampledChannelsLimit, size[2]) - min;    // this is how much we can cap at the current input
                        if (cappingMargin > 0) {
                            depthCapping = std::min(cappingMargin, sampledChannels[dst] - sampledChannelsLimit);
                            // reduce the excess
                            sampledChannels[dst] -= depthCapping;
                        }
                    }

                    // try to recycle an existing storage first
                    for (auto& i : refs) {
                        auto candidate = i.first;
                        auto& users = i.second;
                        const int reservedDepth = sampledChannelsLimit - 4 * candidate->getNumberOfTextures();
                        // check if (1) size matches, (2) padding is sufficient, (3) reserved depth matches the number of channels to cap or no capping
                        if (candidate->getSize() == size && candidate->getPadding() >= dst->getInputPadding(conn.input) && (reservedDepth == depthCapping || depthCapping == 0)
                            && users.empty())
                        {
                            // found!
                            storage = candidate;
                            users.push_back(dst);
                            break;
                        }
                        if (storage)
                            break;
                    }

                    // no matching storage found, allocate a new one
                    if (!storage) {
                        storage = (size[0] == 1 && size[1] == 1) ?
                            // allocate flat storage if the output size is of 1x1 pixels
                            &allocateFlatStorage(gpu, size[2]) :
                            &allocateStorage(gpu,
                                size,
                                src->usesGpu(), !src->usesGpu(),
                                paddings[conn.output],
                                depthCapping
                            );
                        refs.emplace(storage, std::vector<AbstractOperation*>{ dst });
                    }

                    // mark output as allocated
                    outputs[conn.output] = storage;
                }

                // connect
                src->setOutput(*storage, conn.output);
                if (conn.shuffle > 0)
                    dst->setInput(Storage::View(*storage, conn.shuffle), conn.input);
                else
                    dst->setInput(*storage, conn.input);
            }

            // if a Vector is accepted
            else if (src->acceptsVectorOutput(conn.output) && dst->acceptsVectorInput(conn.input)) {
                RuntimeError::check(conn.shuffle == 0, "Cannot shuffle vector");
                GL::Vector* vector;

                // check if the output storage is already allocated
                if (outputs[conn.output])
                    vector = static_cast<GL::Vector*>(outputs[conn.output]);
                else {
                    vector = &allocateVector(gpu, src->getOutputSize(conn.output).volume());
                    outputs[conn.output] = vector;
                }

                // connect
                src->setOutput(*vector, conn.output);
                dst->setInput(*vector, conn.input);
            }

            // if a texture is accepted
            else if (src->acceptsTextureOutput(conn.output) && dst->acceptsTextureInput(conn.input)) {
                RuntimeError::check(conn.shuffle == 0, "Cannot shuffle texture");
                InternalBitmap* texture;

                // check if the output storage is already allocated
                if (outputs[conn.output])
                    texture = static_cast<InternalBitmap*>(outputs[conn.output]);
                else
                    outputs[conn.output] = texture = &allocateTexture(gpu, src->getOutputSize(conn.output));

                // connect
                src->setOutput(*texture, conn.output);
                dst->setInput(*texture, conn.input);
            }

            else
                throw RuntimeError("Cannot connect " + src->getName() + " (output #" + std::to_string(conn.output) + ") "
                    "to " + dst->getName() + " (input #" + std::to_string(conn.input) + "): storage type mismatch");
        }

        // allocate user outputs if not yet
        auto userOutputs = this->userOutputs.equal_range(src);
        for (auto i = userOutputs.first; i != userOutputs.second; ++i) {
            int idx = i->second.index;
            if (idx >= src->getOutputCount())
                throw InvalidArgument("Operation " + src->getName() + " does not have output #" + std::to_string(idx));
            if (!connectedOutputs[idx])
                if (src->acceptsStorageOutput(idx)) {
                    src->setOutput(allocateStorage(gpu, src->getOutputSize(idx), src->usesGpu(), !src->usesGpu()), idx);
                }
                else if (src->acceptsVectorOutput(idx)) {
                    src->setOutput(allocateVector(gpu, src->getOutputSize(idx).volume()), idx);
                }
        }

        // prepare operation
        src->prepare(gpu, data, *this);

        // remove references to storages used by the current operation. This allows their reuse in other connections.
        for (auto& i : refs) {
            auto& users = i.second;
            for (auto op = users.begin(); op != users.end(); )
                if (*op == src)
                    users.erase(op);
                else
                    ++op;
        }

        // advance the progress bar
        preparingProgress();
    }

    data.close();
    ready = true;
}


void Model::execute(TaskThread& thread, GraphicPipeline* gpu) {
    if (gpu)
        gpu->switchMode(GraphicPipeline::Mode::INFERENCE);

    // reset the progress tracker
    inferenceProgress.reset(ops.size());

    // loop through ops
    for (auto op : ops) {
        if (thread.isTaskAborted())
            return;

        // start profiling
        if (thread.isManaging() && profiler)
            (*profiler)(op->getName());

        // run operation
        try {
            if (gpu)
                op->execute(thread, *gpu);
            else
                op->execute(thread);
        } catch (const std::exception& ex) {
            throw InferenceTimeError(*op, ex);
        }

        // get user outputs
        auto userOutputs = this->userOutputs.equal_range(op);
        for (auto it = userOutputs.first; it != userOutputs.second; ++it) {
            int idx = it->second.index;
            auto& data = it->second.data;
            if (gpu)
                if (op->acceptsStorageOutput(idx)) {
                    // get data pointer from storage
                    auto view = op->getOutput(idx);
                    if (!view.getStorage().isUpToDate(ProcessingTarget::CPU))
                        view.getStorage().pull(*gpu);

                    // copy to the vector
                    Storage::Scanner scan(view);
                    scan.move(0, 0);
                    data.resize(view.getSize().volume());
                    for (auto it = data.begin(); it != data.end(); it += view.getDepth()) {
                        scan.fill(it, data.end());
                        ++scan;
                    }
                }
                else if (op->acceptsVectorOutput(idx)) {
                    GL::Vector* vector;
                    op->getOutput(vector, idx);
                    vector->fetch(*gpu, data);
                }
        }

        if (thread.isManaging()) {
            // stop profiler
            if (profiler) {
                gpu->flush();   // wait till GPU is done
                profiler->lap();
            }

            // increase inference progress
            inferenceProgress();
        }
    }
}


bool Model::isOperationInModel(const AbstractOperation& operation) const {
    for (auto op : ops)
        if (op == &operation)
            return true;
    return false;
}


void Model::freeMemory() {
    for (auto storage : storages)
        delete storage;
    storages.clear();
    for (auto vector : vectors)
        delete vector;
    vectors.clear();
    for (auto texture : textures)
        delete texture;
    textures.clear();
}


Storage& Model::allocateStorage(GraphicPipeline& gpu, const Size size, bool forGpu, bool forCpu, const int pad, const int reservedDepth) {
    Storage* storage = new Storage(context, gpu, size, pad, reservedDepth);
    if (forGpu)
        storage->allocate(gpu);
    if (forCpu)
        storage->allocate();
    storages.push_back(storage);
    return *storage;
}


Storage& Model::allocateFlatStorage(GraphicPipeline& gpu, int size) {
    Storage* storage = new Storage(context, gpu, Size(1, 1, size));
    storage->allocate(gpu);
    storages.push_back(storage);
    return *storage;
}


GL::Vector& Model::allocateVector(GraphicPipeline& gpu, const int size) {
    GL::Vector::Format format;
#ifdef BEATMUP_OPENGLVERSION_GLES20
    format = GL::Vector::Format::FIXED16;
#else
    format = GL::Vector::Format::FLOAT;
#endif
    GL::Vector* vector = new GL::Vector(context, gpu, size, format);
    vectors.push_back(vector);
    return *vector;
}


InternalBitmap& Model::allocateTexture(GraphicPipeline& gpu, const Size size) {
    PixelFormat pixelFormat(PixelFormat::TripleByte);
    switch (size.getDepth()) {
    case 1:
        pixelFormat = PixelFormat::SingleByte;
        break;
    case 3:
        pixelFormat = PixelFormat::TripleByte;
        break;
    case 4:
        pixelFormat = PixelFormat::QuadByte;
        break;
    default:
        throw InvalidArgument("Unsupported depth: " + std::to_string(size.getDepth()));
    }
    textures.push_back(new InternalBitmap(context, pixelFormat, size.getWidth(), size.getHeight()));
    return *textures.back();
}


bool Model::isPreceding(const AbstractOperation& first, const AbstractOperation& second) const {
    for (size_t firstIdx = 0; firstIdx < ops.size(); ++firstIdx)
        if (ops[firstIdx] == &first) {
            for (size_t secondIdx = firstIdx + 1; secondIdx < ops.size(); ++secondIdx)
                if (ops[secondIdx] == &second)
                    return true;
            return false;
        }
    return false;
}


AbstractOperation* Model::operator[](const std::string& operationName) {
    for (auto op : ops)
        if (op->getName() == operationName)
            return op;
    throw InvalidArgument("Operation not found: " + operationName);
}


const AbstractOperation* Model::operator[](const std::string& operationName) const {
    for (auto op : ops)
        if (op->getName() == operationName)
            return op;
    throw InvalidArgument("Operation not found: " + operationName);
}


unsigned long Model::countMultiplyAdds() const {
    unsigned long result = 0;
    for (auto op : ops)
        result += op->countMultiplyAdds();
    return result;
}


unsigned long Model::countTexelFetches() const {
    unsigned long result = 0;
    for (auto op : ops)
        result += op->countTexelFetches();
    return result;
}


size_t Model::getMemorySize() const {
    size_t size = 0;
    for (auto& entry : storages)
        size += entry->getMemorySize();
    for (auto& entry : vectors)
        size += entry->getMemorySize();
    for (auto& entry : textures)
        size += entry->getMemorySize();
    return size;
}


Listing Model::serialize() const {
    /** \page NNetsConnectionsSerialization  Connections serialization
        Every connection is serialized in a single block in \c connections part.

        Example:
        \code{yaml}
         - from: source operation name
           to: destination operation name
           from_output: 0       # output number of the source operation, defaults to 0
           to_input: 0          # input number of the destination operation, defaults to 0
           shuffle: 1           # shuffling step, defaults to 1
        \endcode
        Shuffling step description is given \ref NNetsShufflingExplained "here".
    */
    Listing listing;

    // serialize operations
    for (const auto& op : ops)
        listing.emplace("ops", op->serialize());

    // serialize connections
    for (const auto& conn : connections) {
        const auto& info = conn.second;
        Listing::Block block;
        block.set("from", conn.first->getName());
        block.set("to", info.dest->getName());
        if (info.output > 0)
            block.set("from_output", info.output);
        if (info.input > 0)
            block.set("to_input", info.input);
        if (info.shuffle > 0)
            block.set("shuffle", info.shuffle);
        listing.emplace("connections", std::move(block));
    }

    // in case if no connections, add empty block
    if (connections.empty())
        listing.emplace("connections", {});

    return listing;
}


std::string Model::serializeToString() const {
    Listing listing(serialize());
    std::stringstream strstr;
    listing.printOut(strstr);
    return strstr.str();
}


InferenceTimeError::InferenceTimeError(const AbstractOperation& op, const std::exception& ex):
    Exception("Error in %s: %s", op.getName().c_str(), ex.what())
{}

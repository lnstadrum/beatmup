#include "inference.h"
#include "../exception.h"
#include "../utils/bitset.h"
#include <iostream>

using namespace Beatmup;
using namespace NNets;


class Serializer {

private:
    Model::Graph& graph;
    std::vector<size_t>& serialized;
    Utils::Bitset opsDone;

    void recursiveSerialize(size_t opIdx) {
        opsDone.set(opIdx, true);
        for (auto& _ : graph[opIdx]) {
            if (!opsDone[_.operationIndex])
                recursiveSerialize(_.operationIndex);
        }
        serialized.push_back(opIdx);
    }

public:
    Serializer(const Model& model, Model::Graph& inputsGraph, std::vector<size_t>& serialized) :
        graph(inputsGraph), serialized(serialized), opsDone(model.getOperationsCount(), false)
    {
        // find outputs
        const size_t count = model.getOperationsCount();
        Utils::Bitset outs(count, true);
        for (size_t i = 0; i < count; ++i)
            for (const auto& in : inputsGraph[i])
                outs.set(in.operationIndex, false);
        if (!outs.any())
            throw Inference::ModelError(model, "No outputs found in the model to infer");

        // go
        serialized.clear();
        for (size_t i = 0; i < count; ++i)
            if (outs[i] && !opsDone[i]) {
                recursiveSerialize(i);
            }
    }
};


Inference::ModelError::ModelError(const Model& model, const char* message):
    Exception(message)
{}


Inference::Inference():
    model(nullptr),
    prepareOnly(false),
    amountOfWork(1)
{}


Inference::~Inference() {
    clearStorage();
    for (auto& _ : inputs)
        delete _.second;
    for (auto& _ : outputs)
        delete _.second;
}


void Inference::setModel(Model* model) {
    this->model = model;
}


void Inference::setPrepareOnly(bool prepareOnly) {
    this->prepareOnly = prepareOnly;
}


void Inference::clearStorage() {
    for (auto& entry : storage)
        delete entry.second.first;
    storage.clear();
}


void Inference::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    if (!model)
        throw NullTaskInput("No model specified");

    // model preparation, if needed
    if (prepareOnly || usedModelRevision != model->getRevision()) {

        // build graph
        model->buildGraph(modelGraph, Model::GraphKind::BOTH);
        // count output users
        const size_t opCount = model->getOperationsCount();
        outputUserCount.resize(model->getOperationsCount());
        for (size_t i = 0; i < outputUserCount.size(); ++i) {
            outputUserCount[i].clear();
            outputUserCount[i].resize(model->getOperation(i)->getOutputCount(), 0);
            for (const auto& conn : modelGraph[opCount + i]) {
                outputUserCount[i][ conn.sourceOutput ]++;
            }
        }

        // serialize
        Serializer serialize(*model, modelGraph, serializedModel);

        // for progress tracking
        amountOfWork = 0;
        for (size_t opIdx : serializedModel)
            amountOfWork += model->getOperation(opIdx)->getMaxProgress();

        // prepare
        ChunkFile& modelData = model->getData();
        modelData.open();

        // use cache
        if (cacheUsed && ChunkFile::readable(cacheFileName)) {
            std::vector<AbstractOperation*> misses;
            ChunkFile cache(cacheFileName);
            cache.open();
            progress.reset();
            for (size_t opIdx : serializedModel) {
                AbstractOperation* op = model->getOperation(opIdx);
                if (cache.chunkExists(op->getName()))
                    op->load(*gpu, cache, progress);
                else {
                    op->prepare(*gpu, modelData, progress);
                    misses.push_back(op);
                }
            }
            cache.close();

            // write out misses
            ChunkFile::Writer cacheWriter(cacheFileName, true);
            for (auto& op : misses)
                op->store(*gpu, cacheWriter, ProgressTracking::DEVNULL);
        }
        else {
            progress.reset();
            for (size_t opIdx : serializedModel)
                model->getOperation(opIdx)->prepare(*gpu, modelData, progress);
            modelData.close();
            // store to cache
            if (cacheUsed) {
                ChunkFile::Writer cache(cacheFileName);
                progress.reset();
                for (size_t opIdx : serializedModel)
                    model->getOperation(opIdx)->store(*gpu, cache, progress);
            }
        }

        // store model revision number
        usedModelRevision = model->getRevision();
    }
}


void Inference::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    clearStorage();
}


bool Inference::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    // if preparation-only flag is set, anything to do
    if (prepareOnly) {
        prepareOnly = false;
        return true;
    }

    // supply inputs
    for (const auto& in : inputs) {
        AbstractOperation* op = model->getOperation(in.first.first);
        if (!op)
            throw ModelError(*model, "Input not found: " + in.first.first);
        op->setInput(*in.second, in.first.second);
    }

    // supply outputs
    for (const auto& out : outputs) {
        AbstractOperation* op = model->getOperation(out.first.first);
        if (!op)
            throw ModelError(*model, "Output not found: " + out.first.first);
        op->setOutput(*out.second, out.first.second);
    }

    // infer
    for (size_t opIdx : serializedModel) {
        AbstractOperation* op = model->getOperation(opIdx);

        // allocate outputs
        const size_t shift = model->getOperationsCount();
        for (const auto& conn : modelGraph[opIdx + shift]) {
            if (outputUserCount[opIdx][conn.sourceOutput] == 0)
                continue;
            std::pair<size_t, int> key(opIdx, conn.sourceOutput);
            auto it = storage.find(key);

            // if not found, create
            if (it == storage.end()) {
                Storage* buffer = op->allocateOutput(conn.sourceOutput);
                op->setOutput(*buffer, conn.sourceOutput);
                model->getOperation(conn.operationIndex)->setInput(*buffer, conn.destinationInput);
                storage.emplace(key, std::make_pair<>(buffer, outputUserCount[opIdx][conn.sourceOutput]));
            }

            // otherwise use
            else {
                model->getOperation(conn.operationIndex)->setInput(*it->second.first, conn.destinationInput);
            }
        }

        // go
#ifdef BEATMUP_DEBUG
        prof(op->getName());
#endif
        op->perform(gpu);
#ifdef BEATMUP_DEBUG
        prof.lap();
#endif

        // decrease users number / deallocate: run through op inputs
        for (const auto& conn : modelGraph[opIdx]) {
            std::pair<size_t, int> key(conn.operationIndex, conn.sourceOutput);
            auto it = storage.find(key);
            // drop if no more users
            if (it->second.second <= 1) {
                delete it->second.first;
                storage.erase(it);
            }
            else {
                it->second.second--;
            }
        }
    }

    return true;
}


void Inference::supplyInput(AbstractBitmap& bitmap, const std::string& opName, int index) {
    const auto key = std::make_pair<>(opName, index);
    auto it = inputs.find(key);
    if (it != inputs.end()) {
        delete it->second;
        it->second = new Storage(&bitmap);
    }
    else
        inputs.emplace(key, new Storage(&bitmap));
}


void Inference::supplyOutput(AbstractBitmap& bitmap, const std::string& opName, int index) {
    const auto key = std::make_pair<>(opName, index);
    auto it = outputs.find(key);
    if (it != outputs.end()) {
        delete it->second;
        it->second = new Storage(&bitmap);
    }
    else
        outputs.emplace(key, new Storage(&bitmap));
}


void Inference::supplyOutput(GL::StorageBuffer& data, const size_t capacity, const std::string& opName, int index) {
    const auto key = std::make_pair<>(opName, index);
    auto it = outputs.find(key);
    if (it != outputs.end()) {
        delete it->second;
        it->second = new Storage(&data, capacity * sizeof(float));
    }
    else
        outputs.emplace(key, new Storage(&data, capacity * sizeof(float)));
}


void Inference::enableCache(const std::string& filename) {
    cacheFileName = filename;
    cacheUsed = true;
}
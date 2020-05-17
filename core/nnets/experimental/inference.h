/*
    Neural nets inference
*/

#pragma once
#include "model.h"
#include "../../gpu/gpu_task.h"
#include "../../utils/progress_tracking.h"
#ifdef BEATMUP_DEBUG
#include "../../utils/profiler.h"
#endif

namespace Beatmup {
    namespace NNets {

        class Inference : public GpuTask {
        private:
#ifdef BEATMUP_DEBUG
            Profiler prof;
#endif

            Model* model;
            Model::Graph modelGraph;
            bool prepareOnly;
            msize usedModelRevision;

            std::string cacheFileName;
            bool cacheUsed;

            ProgressTracking progress;
            size_t amountOfWork;

            std::vector<std::vector<int>> outputUserCount;
                //!< number of inputs using every operation output, needed for storage allocation

            std::vector<size_t> serializedModel;				//!< sequence of operation indices

            std::map<std::pair<std::string, int>, Storage*>
                inputs,
                outputs;

            std::map<std::pair<size_t, int>, std::pair<Storage*, int>> storage;
                //!< (operation index, output index) mapped to (buffer, its reference count)

            void clearStorage();

            void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
            void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
            bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);

        public:
            class ModelError : public Exception {
            public:
                ModelError(const Model& model, const char* message);
                ModelError(const Model& model, const std::string& message) : ModelError(model, message.c_str()) {}
            };

            Inference();
            ~Inference();

            void setModel(Model* model);
            Model* getModel() const { return model; }

            void setPrepareOnly(bool prepareOnly);

            void supplyInput(AbstractBitmap& bitmap, const std::string& opName, int index = 0);
            void supplyOutput(AbstractBitmap& bitmap, const std::string& opName, int index = 0);
            void supplyOutput(GL::StorageBuffer& data, const size_t capacity, const std::string& opName, int index = 0);

            void enableCache(const std::string& filename);

            float getProgressPercent() const { return 100.0f * progress.getProgress() / amountOfWork; }

#ifdef BEATMUP_DEBUG
            const Profiler& getProfiler() const { return prof; }
#endif
        };

    }
}

#pragma once
#include "abstract_bitmap.h"
#include "../parallelism.h"
#include <vector>

namespace Beatmup {

    class Metric : public AbstractTask {
    public:
        enum class Norm {
            L1,
            L2
        };

    private:
        AbstractBitmap* bitmap[2];
        IntRectangle roi[2];
        Norm norm;
        std::vector<double> results;
        double result;

        inline ThreadIndex maxAllowedThreads() const { return  MAX_THREAD_INDEX; }
        void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
        bool process(TaskThread& thread);

    public:
        Metric();

        void setBitmaps(AbstractBitmap* bitmap1, AbstractBitmap* bitmap2);
        void setBitmaps(AbstractBitmap* bitmap1, const IntRectangle& roi1, AbstractBitmap* bitmap2, const IntRectangle& roi2);
        void setNorm(Norm norm) { this->norm = norm; }

        double getResult() const { return result; }

        static float psnr(AbstractBitmap& bitmap1, AbstractBitmap& bitmap2);
    };
}

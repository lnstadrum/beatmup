#include "metric.h"
#include "processing.h"

using namespace Beatmup;


template<class in_t> class ComputeL1Metric {
public:
    static inline void process(
        AbstractBitmap& bitmap1, AbstractBitmap& bitmap2,
        const IntRectangle& roi1, const IntRectangle& roi2,
        double& result
    ) {
        in_t in1(bitmap1), in2(bitmap2);
        result = 0;
        for (int y = roi1.a.y; y < roi1.b.y; ++y) {
            in1.goTo(roi1.a.x, y);
            in2.goTo(roi2.a.x, roi2.a.y + y - roi1.a.y);
            for (int x = roi2.a.x; x < roi2.b.x; ++x, in1++, in2++) {
                const auto diff = in1() - in2();
                result += diff.abs().makeFloat().sum();
            }
        }
    }
};


template<class in_t> class ComputeSquaredL2Metric {
public:
    static inline void process(
        AbstractBitmap& bitmap1, AbstractBitmap& bitmap2,
        const IntRectangle& roi1, const IntRectangle& roi2,
        double& result
    ) {
        in_t in1(bitmap1), in2(bitmap2);
        result = 0;
        for (int y = roi1.a.y; y < roi1.b.y; ++y) {
            in1.goTo(roi1.a.x, y);
            in2.goTo(roi2.a.x, roi2.a.y + y - roi1.a.y);
            for (int x = roi2.a.x; x < roi2.b.x; ++x, in1++, in2++) {
                const auto diff = (in1() - in2()).makeFloat();
                result += (diff * diff).sum();
            }
        }
    }
};


Metric::Metric(): bitmap{ nullptr, nullptr }, norm(Norm::L2), result(0)
{}


void Metric::setBitmaps(AbstractBitmap* bitmap1, AbstractBitmap* bitmap2) {
    this->bitmap[0] = bitmap1;
    this->bitmap[1] = bitmap2;
    if (this->bitmap[0])
        roi[0] = this->bitmap[0]->getSize().halfOpenedRectangle();
    if (this->bitmap[1])
        roi[1] = this->bitmap[1]->getSize().halfOpenedRectangle();
}


void Metric::setBitmaps(AbstractBitmap* bitmap1, const IntRectangle& roi1, AbstractBitmap* bitmap2, const IntRectangle& roi2) {
    this->bitmap[0] = bitmap1;
    this->bitmap[1] = bitmap2;
    this->roi[0] = roi1;
    this->roi[1] = roi2;
}


void Metric::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(bitmap[0], "bitmap 1");
    NullTaskInput::check(bitmap[1], "bitmap 2");
    RuntimeError::check(bitmap[0]->getPixelFormat() == bitmap[1]->getPixelFormat(), "Pixel format mismatch");
    RuntimeError::check(roi[0] && roi[1], "Regions of interest are of different size");
    bitmap[0]->lockContent(PixelFlow::CpuRead);
    bitmap[1]->lockContent(PixelFlow::CpuRead);
    results.resize(threadCount);
}


void Metric::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    bitmap[0]->unlockContent(PixelFlow::CpuRead);
    bitmap[1]->unlockContent(PixelFlow::CpuRead);
    double sum = 0;
    for (const auto& _ : results)
        sum += _;

    switch (norm) {
    case Norm::L1:
        result = sum;
        break;

    case Norm::L2:
        result = std::sqrt(sum);
        break;
    }
}


bool Metric::process(TaskThread& thread) {
    const IntRectangle
        myRoi1 = roi[0].split(thread.currentThread(), thread.totalThreads()),
        myRoi2 = roi[1].split(thread.currentThread(), thread.totalThreads());

    switch (norm) {
    case Norm::L1:
        BitmapProcessing::write<ComputeL1Metric>(*bitmap[0], *bitmap[1], myRoi1, myRoi2, results[thread.currentThread()]);
        break;

    case Norm::L2:
        BitmapProcessing::write<ComputeSquaredL2Metric>(*bitmap[0], *bitmap[1], myRoi1, myRoi2, results[thread.currentThread()]);
        break;
    }

    return true;
}


float Metric::psnr(AbstractBitmap& bitmap1, AbstractBitmap& bitmap2) {
    Metric metric;
    metric.setBitmaps(&bitmap1, &bitmap2);
    metric.setNorm(Norm::L2);
    bitmap1.getContext().performTask(metric);
    const int n = bitmap1.getSize().numPixels() * bitmap1.getNumberOfChannels();
    return 20 * std::log10(std::sqrt((double)n) / metric.getResult());
}

/*
    Bitmap type converter
*/

#pragma once
#include "abstract_bitmap.h"
#include "../parallelism.h"

namespace Beatmup {

    class BitmapConverter : public AbstractTask {
    private:
        const int MIN_PIXEL_COUNT_PER_THREAD = 1000;		//!< minimum number of pixels per worker
        AbstractBitmap *input, *output;						//!< input and output bitmaps
        
        void doConvert(int outX, int outY, msize nPix);
    protected:
        virtual bool process(TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
        virtual void afterProcessing(ThreadIndex threadCount, bool aborted);
    public:
        BitmapConverter();
        void setBitmaps(AbstractBitmap* input, AbstractBitmap* output);
        ThreadIndex maxAllowedThreads() const;
        ExecutionTarget getExecutionTarget() const;

        static void convert(AbstractBitmap& input, AbstractBitmap& output);
    };

}
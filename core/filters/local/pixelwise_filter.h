/*
    Pixelwise bitmap operation: each pixel value in the output depends only on the same pixel value in the input
*/

#pragma once
#include "../../parallelism.h"
#include "../../bitmap/abstract_bitmap.h"

namespace Beatmup {
    namespace Filters {

        class PixelwiseFilter : public AbstractTask {
        protected:
            const int MIN_PIXEL_COUNT_PER_THREAD = 64 * 64;

            AbstractBitmap *inputBitmap, *outputBitmap;

            /**
                Applies filtering to given pixel data.
                \params startx		horizontal position of the first pixel in image
                \params starty		vertical position of the first pixel in image
                \params nPix		number of pixels to process
                \params thread		current thread info
                */
            virtual void apply(
                int startx,
                int starty,
                msize nPix,
                TaskThread& thread
                ) = 0;

            virtual bool process(TaskThread& thread) final;

            virtual void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) final;

            virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) final;

        public:
            PixelwiseFilter();

            virtual void setBitmaps(AbstractBitmap *input, AbstractBitmap *output);

            ThreadIndex maxAllowedThreads() const;
        };

    }

}

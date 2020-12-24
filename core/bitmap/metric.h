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

#pragma once
#include "abstract_bitmap.h"
#include "../parallelism.h"
#include <vector>

namespace Beatmup {

    /**
        Measures the difference between two bitmaps
    */
    class Metric : public AbstractTask, private BitmapContentLock {
    public:
        /**
            Norm (distance) to measure between two images
        */
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

        inline ThreadIndex getMaxThreads() const { return  MAX_THREAD_INDEX; }
        void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
        bool process(TaskThread& thread);

    public:
        Metric();

        /**
            Sets input images
        */
        void setBitmaps(AbstractBitmap* bitmap1, AbstractBitmap* bitmap2);

        /**
            Sets input images and rectangular regions delimiting the measurement areas
        */
        void setBitmaps(AbstractBitmap* bitmap1, const IntRectangle& roi1, AbstractBitmap* bitmap2, const IntRectangle& roi2);

        /**
            Specifies the norm to use in the measurement
        */
        void setNorm(Norm norm) { this->norm = norm; }

        /**
            \return the measurement result (after the task is executed)
        */
        double getResult() const { return result; }

        /**
            \return peak signal-to-noise ratio in dB for two given images
        */
        static float psnr(AbstractBitmap& bitmap1, AbstractBitmap& bitmap2);
    };
}

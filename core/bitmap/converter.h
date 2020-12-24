/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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

namespace Beatmup {

    /**
        Converts bitmap content from one pixel format to another one.
    */
    class FormatConverter : public AbstractTask, private BitmapContentLock {
    private:
        const int MIN_PIXEL_COUNT_PER_THREAD = 1000;		//!< minimum number of pixels per worker
        AbstractBitmap *input, *output;						//!< input and output bitmaps
        
        void doConvert(int outX, int outY, msize nPix);
    protected:
        virtual bool process(TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
        virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
    public:
        FormatConverter();
        void setBitmaps(AbstractBitmap* input, AbstractBitmap* output);
        ThreadIndex getMaxThreads() const;
        TaskDeviceRequirement getUsedDevices() const;

        static void convert(AbstractBitmap& input, AbstractBitmap& output);
    };

}
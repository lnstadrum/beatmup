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
#include "../parallelism.h"
#include "../geometry.h"
#include "abstract_bitmap.h"

namespace Beatmup {

    /**
        A task to clip images on CPU.
    */
    class Crop : public AbstractTask, private BitmapContentLock {
    private:
        AbstractBitmap *input, *output;			//!< input and output bitmaps
        IntPoint outOrigin;						//!< origin on output bitmap
        IntRectangle cropRect;					//!< clip rect on input bitmap
    protected:
        virtual bool process(TaskThread&);
        virtual void beforeProcessing(ThreadIndex, ProcessingTarget target, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, GraphicPipeline*, bool);
    public:
        Crop();

        ThreadIndex getMaxThreads() const { return 1; }

        void setInput(AbstractBitmap* input);
        void setOutput(AbstractBitmap* output);

        /**
            Sets crop rectangle in input bitmap
        */
        void setCropRect(IntRectangle);

        /**
            Sets top-left position of the clip rectangle in output bitmap
        */
        void setOutputOrigin(IntPoint);

        /**
            Checks if everything is fitted to make cropping
        */
        bool isFit() const;

        /**
            Copies out a specified rect of a bitmap into another bitmap
        */
        static AbstractBitmap* run(AbstractBitmap& bitmap, IntRectangle clipRect);
    };
}

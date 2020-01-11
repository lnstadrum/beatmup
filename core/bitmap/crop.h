/*
    Simple and extremely fast bitmap clip
*/

#pragma once
#include "../parallelism.h"
#include "../geometry.h"
#include "abstract_bitmap.h"

namespace Beatmup {

    class Crop : public AbstractTask {
    private:
        AbstractBitmap *input, *output;			//!< input and output bitmaps
        IntPoint outOrigin;						//!< origin on output bitmap
        IntRectangle cropRect;					//!< clip rect on input bitmap
    protected:
        virtual bool process(TaskThread&);
        virtual void beforeProcessing(ThreadIndex, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, bool);
    public:
        Crop();

        ThreadIndex maxAllowedThreads() const { return 1; }

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
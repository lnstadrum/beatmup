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

#include "abstract_bitmap.h"
#include "../parallelism.h"
#include "../geometry.h"

namespace Beatmup {

    /**
        A binary pixelwise operation on images.
        Evaluates expression O = op(L, R) for bitmaps L, R, O and a given pixelwise operation op.
        Allows to select the operating area in all the three bitmaps.
    */
    class BitmapBinaryOperation : public AbstractTask, private BitmapContentLock {
    public:
        /**
            Binary operation specification.
        */
        enum class Operation {
            NONE,       //!< bypass; the output bitmap remains unchanged
            ADD,        //!< the input images are added
            MULTIPLY    //!< the input images are multiplied
        };

    private:
        const int MIN_PIXEL_COUNT_PER_THREAD = 1000;		//!< minimum number of pixels per worker

        AbstractBitmap *op1, *op2, *output;						//!< input and output bitmaps
        Operation operation;
        IntPoint op1Origin, op2Origin, outputOrigin;
        int cropWidth, cropHeight;

    protected:
        virtual bool process(TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex, ProcessingTarget target, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, GraphicPipeline*, bool);
        virtual ThreadIndex getMaxThreads() const;

    public:
        BitmapBinaryOperation();
        void setOperand1(AbstractBitmap* op1);
        void setOperand2(AbstractBitmap* op2);
        void setOutput(AbstractBitmap* output);
        void setOperation(const Operation operation);
        void setCropSize(int width, int height);
        void setOp1Origin(const IntPoint origin);
        void setOp2Origin(const IntPoint origin);
        void setOutputOrigin(const IntPoint origin);

        void resetCrop();

        int getCropWidth()  const { return cropWidth; }
        int getCropHeight() const { return cropHeight; }
        const IntPoint getOp1Origin() const    { return op1Origin; }
        const IntPoint getOp2Origin() const    { return op2Origin; }
        const IntPoint getOutputOrigin() const { return outputOrigin; }

    };
}

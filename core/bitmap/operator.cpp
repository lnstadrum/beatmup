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

#include "operator.h"
#include "bitmap_access.h"
#include "mask_bitmap_access.h"


using namespace Beatmup;


namespace Kernels {

    template<class in_t, class out_t> class BinaryOpBody {
    public:

        /**
            Performs a binary operation on two bitmaps
        */
        static void process(
                in_t op1, in_t op2, out_t out,
                BitmapBinaryOperation::Operation operation,
                int width, int height,
                const IntPoint& op1Origin,
                const IntPoint& op2Origin,
                const IntPoint& outOrigin,
                const TaskThread& tt
        ) {
            if (operation == BitmapBinaryOperation::Operation::NONE)
                return;

            for (int y = tt.currentThread(); y < height; y += tt.numThreads()) {
                op1.goTo(op1Origin.x, op1Origin.y + y);
                op2.goTo(op2Origin.x, op2Origin.y + y);
                out.goTo(outOrigin.x, outOrigin.y + y);

                switch (operation) {
                    case BitmapBinaryOperation::Operation::ADD:
                        for (int x = 0; x < width; ++x, op1++, op2++, out++) {
                            out = op1() + op2();
                        }
                        break;

                    case BitmapBinaryOperation::Operation::MULTIPLY:
                        for (int x = 0; x < width; ++x, op1++, op2++, out++) {
                            out = op1() * op2();
                        }
                        break;

                    default: return;
                }

                if (tt.isTaskAborted())
                    return;
            }
        }


        /**
            Performs a binary operation on two binary masks
        */
        static void processAlignedBinaryMask(
                in_t op1, in_t op2, out_t out,
                BitmapBinaryOperation::Operation operation,
                int width, int height,
                const IntPoint &op1Origin,
                const IntPoint &op2Origin,
                const IntPoint &outOrigin,
                const TaskThread &tt
        ) {
            if (operation == BitmapBinaryOperation::Operation::NONE)
                return;

            BEATMUP_ASSERT_DEBUG(op1Origin.x % 8 == 0);
            BEATMUP_ASSERT_DEBUG(op2Origin.x % 8 == 0);
            BEATMUP_ASSERT_DEBUG(outOrigin.x % 8 == 0);

            for (int y = tt.currentThread(); y < height; y += tt.numThreads()) {
                int x = 0;

                // running aligned part first
                op1.goTo(op1Origin.x, op1Origin.y + y);
                op2.goTo(op2Origin.x, op2Origin.y + y);
                out.goTo(outOrigin.x, outOrigin.y + y);

                pixint_platform
                        *p1 = (pixint_platform*)*op1,
                        *p2 = (pixint_platform*)*op2,
                        *po = (pixint_platform*)*out;

                static const int step = 8 * sizeof(pixint_platform);

                switch (operation) {
                    case BitmapBinaryOperation::Operation::ADD:
                        for (x = 0; x + step <= width; x += step, p1++, p2++, po++) {
                            *po = *p1 | *p2;
                        }
                        break;

                    case BitmapBinaryOperation::Operation::MULTIPLY:
                        for (x = 0; x + step <= width; x += step, p1++, p2++, po++) {
                            *po = *p1 & *p2;
                        }
                        break;

                    default: return;
                }

                // if unaligned reminder, deal with it
                if (x < width) {
                    op1.goTo(op1Origin.x + x, op1Origin.y + y);
                    op2.goTo(op2Origin.x + x, op2Origin.y + y);
                    out.goTo(outOrigin.x + x, outOrigin.y + y);

                    switch (operation) {
                        case BitmapBinaryOperation::Operation::ADD:
                            for (; x < width; ++x, op1++, op2++, out++) {
                                out = op1() + op2();
                            }
                            break;

                        case BitmapBinaryOperation::Operation::MULTIPLY:
                            for (; x < width; ++x, op1++, op2++, out++) {
                                out = op1() * op2();
                            }
                            break;

                        default: return;
                    }

                }

                if (tt.isTaskAborted())
                    return;
            }
        }
    };
}


BitmapBinaryOperation::BitmapBinaryOperation() :
    op1(nullptr), op2(nullptr), output(nullptr),
    operation(Operation::NONE)
{}


void BitmapBinaryOperation::setOperand1(AbstractBitmap* bitmap) {
    this->op1 = bitmap;
}


void BitmapBinaryOperation::setOperand2(AbstractBitmap* bitmap) {
    this->op2 = bitmap;
}


void BitmapBinaryOperation::setOutput(AbstractBitmap* bitmap) {
    this->output = bitmap;
}


void BitmapBinaryOperation::setOperation(const Operation operation) {
    this->operation = operation;
}


void BitmapBinaryOperation::setCropSize(int width, int height) {
    this->cropWidth = width;
    this->cropHeight = height;
}


void BitmapBinaryOperation::setOp1Origin(const IntPoint origin) {
    this->op1Origin = origin;
}


void BitmapBinaryOperation::setOp2Origin(const IntPoint origin) {
    this->op2Origin = origin;
}


void BitmapBinaryOperation::setOutputOrigin(const IntPoint origin) {
    this->outputOrigin = origin;
}


void BitmapBinaryOperation::resetCrop() {
    if (op1 && op2 && output) {
        cropWidth  = std::min(output->getWidth(),  std::min(op1->getWidth(),  op2->getWidth()));
        cropHeight = std::min(output->getHeight(), std::min(op1->getHeight(), op2->getHeight()));
        op1Origin = op2Origin = outputOrigin = IntPoint::ZERO;
    }
}


ThreadIndex BitmapBinaryOperation::getMaxThreads() const {
    return AbstractTask::validThreadCount(cropHeight / MIN_PIXEL_COUNT_PER_THREAD);
}


void BitmapBinaryOperation::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    NullTaskInput::check(op1, "operand 1 bitmap");
    NullTaskInput::check(op2, "operand 2 bitmap");
    NullTaskInput::check(output, "output bitmap");
    RuntimeError::check(op1->getPixelFormat() == op2->getPixelFormat(), "operands pixel formats do not match");
    RuntimeError::check(op1->getPixelFormat() == output->getPixelFormat(), "output pixel format does not match operands pixel format");
    RuntimeError::check(op1Origin.x + cropWidth  <= op1->getWidth(),  "operand 1 width exceeded");
    RuntimeError::check(op1Origin.y + cropHeight <= op1->getHeight(), "operand 1 height exceeded");
    RuntimeError::check(op2Origin.x + cropWidth  <= op2->getWidth(),  "operand 2 width exceeded");
    RuntimeError::check(op2Origin.y + cropHeight <= op2->getHeight(), "operand 2 height exceeded");
    RuntimeError::check(outputOrigin.x + cropWidth  <= output->getWidth(),  "operand 1 width exceeded");
    RuntimeError::check(outputOrigin.y + cropHeight <= output->getHeight(), "operand 1 height exceeded");


    lock<ProcessingTarget::CPU>(gpu, { op1, op2 }, { output });
}


void BitmapBinaryOperation::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    unlock(output, op1, op2);
}


bool BitmapBinaryOperation::process(TaskThread& thread) {
#define PROCESS(IN_T, OUT_T) \
        Kernels::BinaryOpBody<IN_T, OUT_T>::process( \
            IN_T(*op1), IN_T(*op2), OUT_T(*output), \
            operation, cropWidth, cropHeight, op1Origin, op2Origin, outputOrigin, thread);

    switch (output->getPixelFormat()) {
        case SingleByte:
            PROCESS(SingleByteBitmapReader, SingleByteBitmapWriter);
            break;
        case TripleByte:
            PROCESS(TripleByteBitmapReader, TripleByteBitmapWriter);
            break;
        case QuadByte:
            PROCESS(QuadByteBitmapReader, QuadByteBitmapWriter);
            break;
        case SingleFloat:
            PROCESS(SingleFloatBitmapReader, SingleFloatBitmapWriter);
            break;
        case TripleFloat:
            PROCESS(TripleFloatBitmapReader, TripleFloatBitmapWriter);
            break;
        case QuadFloat:
            PROCESS(QuadFloatBitmapReader, QuadFloatBitmapWriter);
            break;
        case BinaryMask:
            if (op1Origin.x % 8 == 0 && op2Origin.x % 8 == 0 && outputOrigin.x % 8 == 0)
                Kernels::BinaryOpBody<BinaryMaskReader, BinaryMaskWriter>::processAlignedBinaryMask(
                        BinaryMaskReader(*op1), BinaryMaskReader(*op2), BinaryMaskWriter(*output),
                        operation, cropWidth, cropHeight, op1Origin, op2Origin, outputOrigin,
                        thread);
            else
                PROCESS(BinaryMaskReader, BinaryMaskWriter);
            break;
        case QuaternaryMask:
            PROCESS(QuaternaryMaskReader, QuaternaryMaskWriter);
            break;
        case HexMask:
            PROCESS(HexMaskReader, HexMaskWriter);
            break;
    }
    return true;
}

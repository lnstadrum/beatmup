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

#include "../context.h"
#include "../bitmap/converter.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/mask_bitmap_access.h"
#include "../exception.h"
#include <algorithm>
#include <cstring>

using namespace Beatmup;


template<class in_t, class out_t> inline void convertBlock(AbstractBitmap& input, AbstractBitmap& output, int startx, int starty, msize nPix) {
    in_t in(input, startx, starty);
    out_t out(output, startx, starty);
    for (msize n = 0; n < nPix; ++n, in++, out++)
        out = in();
}


FormatConverter::FormatConverter() :
    input(nullptr), output(nullptr)
{}


void FormatConverter::setBitmaps(AbstractBitmap* input, AbstractBitmap* output) {
    this->input = input;
    this->output = output;
}


ThreadIndex FormatConverter::getMaxThreads() const {
    return AbstractTask::validThreadCount((int)(output->getSize().numPixels() / MIN_PIXEL_COUNT_PER_THREAD));
}


AbstractTask::TaskDeviceRequirement FormatConverter::getUsedDevices() const {
    // it does not make sense to convert bitmaps on GPU
    return TaskDeviceRequirement::CPU_ONLY;
}


void FormatConverter::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    NullTaskInput::check(input, "input bitmap");
    NullTaskInput::check(output, "output bitmap");
    RuntimeError::check(input->getSize() == output->getSize(),
        "Input and output bitmap must be of the same size.");
    lock<ProcessingTarget::CPU>(gpu, input, output);
}


void FormatConverter::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    unlock(input, output);
}


bool FormatConverter::process(TaskThread& thread) {
    // if the bitmaps are equal, say done
    if (input == output)
        return true;

    // if pixel formats are identical, just copy
    if (input->getPixelFormat() == output->getPixelFormat()) {
        if (thread.currentThread() == 0)
            memcpy(
                output->getData(0,0),
                input->getData(0,0),
                input->getMemorySize()
            );
        return true;
    }

    // compute splitting accordingly to the current thread index
    int w = output->getWidth();
    msize
        npix = w * output->getHeight(),
        start = npix * thread.currentThread() / thread.numThreads(),
        stop = npix * (1 + thread.currentThread()) / thread.numThreads();

    // computing initial position
    int outx = (int)(start % w), outy = (int)(start / w);

    const int LOOK_AROUND_INTERVAL = 123456;
    while (start < stop && !thread.isTaskAborted()) {
        doConvert(outx, outy, stop-start);
        start += LOOK_AROUND_INTERVAL;
    }

    return true;
}


void FormatConverter::doConvert(int outX, int outY, msize nPix) {
#define CALL_CONVERT_AND_RETURN(IN_T, OUT_T) \
    convertBlock < IN_T, OUT_T >(*input, *output, outX, outY, nPix); return;

    // for all (left) possible pairs of pixel formats, do the conversion
    switch (input->getPixelFormat()) {
    case SingleByte:
        switch (output->getPixelFormat()) {
            case TripleByte:	CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, TripleFloatBitmapWriter)
            case QuadFloat:		CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, QuadFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case TripleByte:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, SingleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, TripleFloatBitmapWriter)
            case QuadFloat:		CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, QuadFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case QuadByte:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, TripleByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, TripleFloatBitmapWriter)
            case QuadFloat:		CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, QuadFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case SingleFloat:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, QuadByteBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, TripleFloatBitmapWriter)
            case QuadFloat:		CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, QuadFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case TripleFloat:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, SingleFloatBitmapWriter)
            case QuadFloat:		CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, QuadFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case QuadFloat:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, TripleFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case BinaryMask:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(BinaryMaskReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, TripleFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case QuaternaryMask:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, TripleFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }

    case HexMask:
        switch (output->getPixelFormat()) {
            case SingleByte:	CALL_CONVERT_AND_RETURN(HexMaskReader, SingleByteBitmapWriter)
            case TripleByte:	CALL_CONVERT_AND_RETURN(HexMaskReader, TripleByteBitmapWriter)
            case QuadByte:		CALL_CONVERT_AND_RETURN(HexMaskReader, QuadByteBitmapWriter)
            case SingleFloat:	CALL_CONVERT_AND_RETURN(HexMaskReader, SingleFloatBitmapWriter)
            case TripleFloat:	CALL_CONVERT_AND_RETURN(HexMaskReader, TripleFloatBitmapWriter)
            default: throw ImplementationUnsupported("Cannot convert given formats");
        }
    }
}


void FormatConverter::convert(AbstractBitmap& input, AbstractBitmap& output) {
    FormatConverter me;
    me.setBitmaps(&input, &output);

    if (input.getContext().isManagingThread()) {
        RuntimeError::check(input.getSize() == output.getSize(),
            "Input and output bitmap must be of the same size.");

        // the bitmaps are assumed locked in this case
        me.doConvert(0, 0, input.getSize().numPixels());

    }
    else
        input.getContext().performTask(me);
}

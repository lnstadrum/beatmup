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
#include "../exception.h"
#include "abstract_bitmap.h"
#include "bitmap_access.h"
#include "mask_bitmap_access.h"

namespace Beatmup {
    /**
        Contains templates calling elementary image processing routines depending on pixel formats of their arguments.
        An elementary routine is a class template having a public function process() performing a specific processing action.
        The template arguments of this class are readers of / writers to bitmaps specialized for given pixel formats.
    */
    namespace BitmapProcessing {
        /**
            Exception thrown in a situation when a processing action is not implemented for pixel formats of specific arguments.
        */
        class ProcessingActionNotImplemented : public Exception {
        public:
            ProcessingActionNotImplemented(PixelFormat fmt) :
                Exception("Processing action is not implemented for given pixel format: '%s'", AbstractBitmap::PIXEL_FORMAT_NAMES[fmt])
            {}
        };


        /**
            Calls a Func< ReaderClass >::process(access, params), where
            - access is an instance of a proper ReaderClass to read `bitmap` starting from pixel (x0, y0),
            - params are additional arguments.
        */
        template<template<class> class Func, class Bitmap, typename... Args>
        inline void read(Bitmap& bitmap, Args&&... args) {
            switch (bitmap.getPixelFormat()) {
            case SingleByte:
                Func<SingleByteBitmapReader>::process(bitmap, args...);
                break;
            case TripleByte:
                Func<TripleByteBitmapReader>::process(bitmap, args...);
                break;
            case QuadByte:
                Func<QuadByteBitmapReader>::process(bitmap, args...);
                break;
            case SingleFloat:
                Func<SingleFloatBitmapReader>::process(bitmap, args...);
                break;
            case TripleFloat:
                Func<TripleFloatBitmapReader>::process(bitmap, args...);
                break;
            case QuadFloat:
                Func<QuadFloatBitmapReader>::process(bitmap, args...);
                break;
            case BinaryMask:
                Func<BinaryMaskReader>::process(bitmap, args...);
                break;
            case QuaternaryMask:
                Func<QuaternaryMaskReader>::process(bitmap, args...);
                break;
            case HexMask:
                Func<HexMaskReader>::process(bitmap, args...);
                break;
            default:
                throw ProcessingActionNotImplemented(bitmap.getPixelFormat());
            }
        }


        /**
            Calls a Func< WriterClass >::process(access, params) that writes to a bitmap of any kind, where
            - access is an instance of a proper WriterClass to write to `bitmap` starting from pixel (x0, y0),
            - params are additional arguments.
        */
        template<template<class> class Func, class Bitmap, typename... Args>
        inline void write(Bitmap& bitmap, Args&&... args) {
            switch (bitmap.getPixelFormat()) {
            case SingleByte:
                Func<SingleByteBitmapWriter>::process(bitmap, args...);
                break;
            case TripleByte:
                Func<TripleByteBitmapWriter>::process(bitmap, args...);
                break;
            case QuadByte:
                Func<QuadByteBitmapWriter>::process(bitmap, args...);
                break;
            case SingleFloat:
                Func<SingleFloatBitmapWriter>::process(bitmap, args...);
                break;
            case TripleFloat:
                Func<TripleFloatBitmapWriter>::process(bitmap, args...);
                break;
            case QuadFloat:
                Func<QuadFloatBitmapWriter>::process(bitmap, args...);
                break;
            case BinaryMask:
                Func<BinaryMaskWriter>::process(bitmap, args...);
                break;
            case QuaternaryMask:
                Func<QuaternaryMaskWriter>::process(bitmap, args...);
                break;
            case HexMask:
                Func<HexMaskWriter>::process(bitmap, args...);
                break;
            default:
                throw ProcessingActionNotImplemented(bitmap.getPixelFormat());
            }
        }


        /**
            Calls a Func< WriterClass >::process(access, params) that writes to a mask bitmap where
            - access is an instance of a proper WriterClass to write to `mask` starting from pixel (x0, y0),
            - params are additional arguments.
        */
        template<template<class> class Func, class Bitmap, typename... Args>
        inline void writeToMask(Bitmap& bitmap, Args&&... args) {
            switch (bitmap.getPixelFormat()) {
            case BinaryMask:
                Func<BinaryMaskWriter>::process(bitmap, args...);
                break;
            case QuaternaryMask:
                Func<QuaternaryMaskWriter>::process(bitmap, args...);
                break;
            case HexMask:
                Func<HexMaskWriter>::process(bitmap, args...);
                break;
            case SingleByte:
                Func<SingleByteMaskWriter>::process(bitmap, args...);
                break;
            default:
                throw ProcessingActionNotImplemented(bitmap.getPixelFormat());
            }
        }


        template<template<class, class> class Func, class InputBitmap, class OutputBitmap, typename... Args>
        inline void pipeline(InputBitmap& in, OutputBitmap& out, Args&&... args) {

#define WRITING(IN_R)  \
            switch (out.getPixelFormat()) { \
                case SingleByte: \
                    Func<IN_R, SingleByteBitmapWriter>::process(in, out, args...); \
                    break; \
                case TripleByte: \
                    Func<IN_R, TripleByteBitmapWriter>::process(in, out, args...); \
                    break; \
                case QuadByte: \
                    Func<IN_R, QuadByteBitmapWriter>::process(in, out, args...); \
                    break; \
                case SingleFloat: \
                    Func<IN_R, SingleFloatBitmapWriter>::process(in, out, args...); \
                    break; \
                case TripleFloat: \
                    Func<IN_R, TripleFloatBitmapWriter>::process(in, out, args...); \
                    break; \
                case QuadFloat: \
                    Func<IN_R, QuadFloatBitmapWriter>::process(in, out, args...); \
                    break; \
                case BinaryMask: \
                    Func<IN_R, BinaryMaskWriter>::process(in, out, args...); \
                    break; \
                case QuaternaryMask: \
                    Func<IN_R, QuaternaryMaskWriter>::process(in, out, args...); \
                    break; \
                case HexMask: \
                    Func<IN_R, HexMaskWriter>::process(in, out, args...); \
                    break; \
                default: throw ProcessingActionNotImplemented(out.getPixelFormat()); \
            }

            switch (in.getPixelFormat()) {
            case SingleByte:
                WRITING(SingleByteBitmapReader);
                break;
            case TripleByte:
                WRITING(TripleByteBitmapReader);
                break;
            case QuadByte:
                WRITING(QuadByteBitmapReader);
                break;
            case SingleFloat:
                WRITING(SingleFloatBitmapReader);
                break;
            case TripleFloat:
                WRITING(TripleFloatBitmapReader);
                break;
            case QuadFloat:
                WRITING(QuadFloatBitmapReader);
                break;
            case BinaryMask:
                WRITING(BinaryMaskReader);
                break;
            case QuaternaryMask:
                WRITING(QuaternaryMaskReader);
                break;
            case HexMask:
                WRITING(HexMaskReader);
                break;
            default:
                throw ProcessingActionNotImplemented(in.getPixelFormat());
            }
#undef WRITING
        }


        template<template<class, class> class Func, class InputBitmap, class OutputBitmap, typename... Args>
        inline void pipelineWithMaskOutput(InputBitmap& in, OutputBitmap& out, Args&&... args) {

#define WRITING(IN_R)  \
            switch (out.getPixelFormat()) { \
                case BinaryMask: \
                    Func<IN_R, BinaryMaskWriter>::process(in, out, args...); \
                    break; \
                case QuaternaryMask: \
                    Func<IN_R, QuaternaryMaskWriter>::process(in, out, args...); \
                    break; \
                case HexMask: \
                    Func<IN_R, HexMaskWriter>::process(in, out, args...); \
                    break; \
                case SingleByte: \
                    Func<IN_R, SingleByteMaskWriter>::process(in, out, args...); \
                    break; \
                default: throw ProcessingActionNotImplemented(out.getPixelFormat()); \
            }

            switch (in.getPixelFormat()) {
            case SingleByte:
                WRITING(SingleByteBitmapReader);
                break;
            case TripleByte:
                WRITING(TripleByteBitmapReader);
                break;
            case QuadByte:
                WRITING(QuadByteBitmapReader);
                break;
            case SingleFloat:
                WRITING(SingleFloatBitmapReader);
                break;
            case TripleFloat:
                WRITING(TripleFloatBitmapReader);
                break;
            case QuadFloat:
                WRITING(QuadFloatBitmapReader);
                break;
            case BinaryMask:
                WRITING(BinaryMaskReader);
                break;
            case QuaternaryMask:
                WRITING(QuaternaryMaskReader);
                break;
            case HexMask:
                WRITING(HexMaskReader);
                break;
            default:
                throw ProcessingActionNotImplemented(in.getPixelFormat());
            }
#undef WRITING
        }
    }
}

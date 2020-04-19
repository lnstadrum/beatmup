#pragma once
#include "../exception.h"
#include "abstract_bitmap.h"
#include "bitmap_access.h"
#include "mask_bitmap_access.h"

namespace Beatmup {
    namespace BitmapProcessing {
        class ProcessingActionNotImplemented : public Exception {
        public:
            ProcessingActionNotImplemented(PixelFormat fmt) :
                Exception("Processing action is not implemented for given pixel fomat: '%s'", AbstractBitmap::PIXEL_FORMAT_NAMES[fmt])
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

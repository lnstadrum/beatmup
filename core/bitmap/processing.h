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
		inline void read(Bitmap& bitmap, int x0, int y0, Args&&... args) {
			switch (bitmap.getPixelFormat()) {
			case SingleByte:
				Func<SingleByteBitmapReader>::process(SingleByteBitmapReader(bitmap, x0, y0), args...);
				break;
			case TripleByte:
				Func<TripleByteBitmapReader>::process(TripleByteBitmapReader(bitmap, x0, y0), args...);
				break;
			case QuadByte:
				Func<QuadByteBitmapReader>::process(QuadByteBitmapReader(bitmap, x0, y0), args...);
				break;
			case SingleFloat:
				Func<SingleFloatBitmapReader>::process(SingleFloatBitmapReader(bitmap, x0, y0), args...);
				break;
			case TripleFloat:
				Func<TripleFloatBitmapReader>::process(TripleFloatBitmapReader(bitmap, x0, y0), args...);
				break;
			case QuadFloat:
				Func<QuadFloatBitmapReader>::process(QuadFloatBitmapReader(bitmap, x0, y0), args...);
				break;
			case BinaryMask:
				Func<BinaryMaskReader>::process(BinaryMaskReader(bitmap, x0, y0), args...);
				break;
			case QuaternaryMask:
				Func<QuaternaryMaskReader>::process(QuaternaryMaskReader(bitmap, x0, y0), args...);
				break;
			case HexMask:
				Func<HexMaskReader>::process(HexMaskReader(bitmap, x0, y0), args...);
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
		inline void write(Bitmap& bitmap, int x0, int y0, Args&&... args) {
			switch (bitmap.getPixelFormat()) {
			case SingleByte:
				Func<SingleByteBitmapWriter>::process(SingleByteBitmapWriter(bitmap, x0, y0), args...);
				break;
			case TripleByte:
				Func<TripleByteBitmapWriter>::process(TripleByteBitmapWriter(bitmap, x0, y0), args...);
				break;
			case QuadByte:
				Func<QuadByteBitmapWriter>::process(QuadByteBitmapWriter(bitmap, x0, y0), args...);
				break;
			case SingleFloat:
				Func<SingleFloatBitmapWriter>::process(SingleFloatBitmapWriter(bitmap, x0, y0), args...);
				break;
			case TripleFloat:
				Func<TripleFloatBitmapWriter>::process(TripleFloatBitmapWriter(bitmap, x0, y0), args...);
				break;
			case QuadFloat:
				Func<QuadFloatBitmapWriter>::process(QuadFloatBitmapWriter(bitmap, x0, y0), args...);
				break;
			case BinaryMask:
				Func<BinaryMaskWriter>::process(BinaryMaskWriter(bitmap, x0, y0), args...);
				break;
			case QuaternaryMask:
				Func<QuaternaryMaskWriter>::process(QuaternaryMaskWriter(bitmap, x0, y0), args...);
				break;
			case HexMask:
				Func<HexMaskWriter>::process(HexMaskWriter(bitmap, x0, y0), args...);
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
		inline void writeToMask(Bitmap& mask, int x0, int y0, Args&&... args) {
			switch (mask.getPixelFormat()) {
			case BinaryMask:
				Func<BinaryMaskWriter>::process(BinaryMaskWriter(mask, x0, y0), args...);
				break;
			case QuaternaryMask:
				Func<QuaternaryMaskWriter>::process(QuaternaryMaskWriter(mask, x0, y0), args...);
				break;
			case HexMask:
				Func<HexMaskWriter>::process(HexMaskWriter(mask, x0, y0), args...);
				break;
			case SingleByte:
				Func<SingleByteMaskWriter>::process(SingleByteMaskWriter(mask, x0, y0), args...);
				break;
			default:
				throw ProcessingActionNotImplemented(mask.getPixelFormat());
			}
		}


		template<template<class, class> class Func, class InputBitmap, class OutputBitmap, typename... Args>
		inline void pipeline(InputBitmap& in, OutputBitmap& out, int x0, int y0, Args&&... args) {

#define WRITING(IN_R)  \
			switch (out.getPixelFormat()) { \
				case SingleByte: \
					Func<IN_R, SingleByteBitmapWriter>::process(IN_R(in,x0,y0), SingleByteBitmapWriter(out,x0,y0), args...); \
					break; \
				case TripleByte: \
					Func<IN_R, TripleByteBitmapWriter>::process(IN_R(in,x0,y0), TripleByteBitmapWriter(out,x0,y0), args...); \
					break; \
				case QuadByte: \
					Func<IN_R, QuadByteBitmapWriter>::process(IN_R(in,x0,y0), QuadByteBitmapWriter(out,x0,y0), args...); \
					break; \
				case SingleFloat: \
					Func<IN_R, SingleFloatBitmapWriter>::process(IN_R(in,x0,y0), SingleFloatBitmapWriter(out,x0,y0), args...); \
					break; \
				case TripleFloat: \
					Func<IN_R, TripleFloatBitmapWriter>::process(IN_R(in,x0,y0), TripleFloatBitmapWriter(out,x0,y0), args...); \
					break; \
				case QuadFloat: \
					Func<IN_R, QuadFloatBitmapWriter>::process(IN_R(in,x0,y0), QuadFloatBitmapWriter(out,x0,y0), args...); \
					break; \
				case BinaryMask: \
					Func<IN_R, BinaryMaskWriter>::process(IN_R(in,x0,y0), BinaryMaskWriter(out,x0,y0), args...); \
					break; \
				case QuaternaryMask: \
					Func<IN_R, QuaternaryMaskWriter>::process(IN_R(in,x0,y0), QuaternaryMaskWriter(out,x0,y0), args...); \
					break; \
				case HexMask: \
					Func<IN_R, HexMaskWriter>::process(IN_R(in,x0,y0), HexMaskWriter(out,x0,y0), args...); \
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
		inline void pipelineWithMaskOutput(InputBitmap& in, OutputBitmap& out, int x0, int y0, Args&&... args) {

#define WRITING(IN_R)  \
			switch (out.getPixelFormat()) { \
				case BinaryMask: \
					Func<IN_R, BinaryMaskWriter>::process(IN_R(in,x0,y0), BinaryMaskWriter(out,x0,y0), args...); \
					break; \
				case QuaternaryMask: \
					Func<IN_R, QuaternaryMaskWriter>::process(IN_R(in,x0,y0), QuaternaryMaskWriter(out,x0,y0), args...); \
					break; \
				case HexMask: \
					Func<IN_R, HexMaskWriter>::process(IN_R(in,x0,y0), HexMaskWriter(out,x0,y0), args...); \
					break; \
				case SingleByte: \
					Func<IN_R, SingleByteMaskWriter>::process(IN_R(in,x0,y0), SingleByteMaskWriter(out,x0,y0), args...); \
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

/*
	Internally managed bitmap
*/

#pragma once

#include "../basic_types.h"
#include "../environment.h"
#include "abstract_bitmap.h"
#include <vector>

namespace Beatmup {

	class InternalBitmap : public AbstractBitmap {
	private:
		PixelFormat pixelFormat;
		int width, height;
		memchunk memory;				//!< ID of memory chunk
		pixptr data;					//!< pointer to pixel data when locked

		void lockPixelData();
		void unlockPixelData();

	public:
		/**
			Creates an emtpy bitmap using internal memory management utilities
		*/
		InternalBitmap(Environment& env, PixelFormat pixelFormat, int width, int height);
		virtual ~InternalBitmap();

		const PixelFormat getPixelFormat() const;
		const int getWidth() const;
		const int getHeight() const;
		const msize getMemorySize() const;
		const pixptr getData(int x, int y) const;
		void unlockPixels();
	};

}
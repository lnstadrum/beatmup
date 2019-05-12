#pragma once

#include "../abstract_bitmap.h"
#ifndef BEATMUP_PLATFORM_WINDOWS
	#error GDI bitmap is Windows-specific.
#endif

namespace Beatmup {

	/**
		A simple wrapper of GDI bitmap
	*/
	class GDIBitmap : public AbstractBitmap {
	private:
		class Impl;
		Impl* impl;

	public:
		GDIBitmap(Environment &env, const wchar_t* filename);

		GDIBitmap(Environment &env, PixelFormat format, int width, int height);

		const PixelFormat getPixelFormat() const;

		const int getWidth() const;

		const int getHeight() const;

		int getLinesAlignment() const;

		const msize getMemorySize() const;

		void lockPixelData();

		void unlockPixelData();

		const pixptr getData(int x, int y) const;

		void save(const wchar_t* filename);
	};

}
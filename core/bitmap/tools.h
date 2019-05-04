/*
	Useful utilities to work with bitmaps
*/

#pragma once
#include "abstract_bitmap.h"
#include "internal_bitmap.h"
#include "../geometry.h"
#include "pixel_arithmetic.h"

namespace Beatmup {
	class BitmapTools : public Object {
	private:
		Environment& env;

	public:
		BitmapTools(Environment& env);

		const Environment& getEnvironment() const;
	
		/**
			\brief Makes a copy of a bitmap with a specified pixel format
		*/
		BitmapPtr makeCopy(AbstractBitmap& source, PixelFormat newPixelFormat);
		
		/**
			\brief Makes a copy of a bitmap
		*/
		BitmapPtr makeCopy(AbstractBitmap& source);

		/**
			Generates a chessboard
		*/
		BitmapPtr chessboard(int width, int height, int cellSize, PixelFormat pixelFormat = BinaryMask);
				
		/**
			Makes a bitmap area opaque
		*/
		static void makeOpaque(AbstractBitmap&, IntRectangle);


		/**
		 * Searches for a pixel of a given value in scaline order starting from a given point
		 * @param source 		the bitmap to look in
		 * @param val 			the pixel value to look for
		 * @param startFrom 	the starting position
		 * @return the next closest position of the searched value (in scaline order) or (-1,-1) if
		 * not found.
		 */
		IntPoint scanlineSearch(AbstractBitmap& source, pixint4 val, const IntPoint& startFrom);
		IntPoint scanlineSearch(AbstractBitmap& source, pixfloat4 val, const IntPoint& startFrom);
	};

}
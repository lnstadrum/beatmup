/*
	Some useful definitions related to the color
*/

#pragma once
#include "../bitmap/pixel_arithmetic.h"

namespace Beatmup{
	typedef pixint4 IntColor;

	namespace IntColors {
		const IntColor
			White				= {	255, 255, 255, 255 },
			Black				= {	0, 0, 0, 255 },
			DarkSeaGreen1		= { 193, 255, 193, 255 },
			DarkSeaGreen2		= { 180, 238, 180, 255 },
			TransparentBlack	= { 0, 0, 0, 0 };
	}

}
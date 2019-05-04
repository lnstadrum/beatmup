/*
	Simple sepia
*/

#pragma once
#include "pixelwise_filter.h"

namespace Beatmup {
	namespace Filters {

		class Sepia : public PixelwiseFilter {
		protected:
			virtual void apply(int startx, int starty, msize nPix, TaskThread& thread);
		};

	}
}
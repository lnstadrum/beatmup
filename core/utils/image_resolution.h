/**
	An entity to handle the resolution
*/

#pragma once
#include "../basic_types.h"
#include "../geometry.h"

namespace Beatmup {
	class ImageResolution {
	public:
		unsigned int width, height;			//!< width and height in pixels
		
		ImageResolution();
		ImageResolution(unsigned int width, unsigned int height);
		
		bool operator==(const ImageResolution&) const;
		bool operator!=(const ImageResolution&) const;
		msize numPixels() const;
		float megaPixels() const;
		float getAspectRatio() const;
		float getInvAspectRatio() const;
		bool fat() const;
		IntRectangle clientRect() const;
	};
}
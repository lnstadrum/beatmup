/**
    An entity to handle the resolution
*/

#pragma once
#include "../basic_types.h"
#include "../geometry.h"


namespace Beatmup {
    class ImageResolution {
    private:
        unsigned int width, height;			//!< width and height in pixels
    public:
        ImageResolution();
        ImageResolution(unsigned int width, unsigned int height);

        bool operator==(const ImageResolution&) const;
        bool operator!=(const ImageResolution&) const;
        msize numPixels() const;
        float megaPixels() const;
        float getAspectRatio() const;
        float getInvAspectRatio() const;
        bool fat() const;

        IntRectangle closedRectangle() const{
            return IntRectangle(0, 0, width - 1, height - 1);
        }
        IntRectangle halfOpenedRectangle() const {
            return IntRectangle(0, 0, width, height);
        }

        unsigned int getWidth() const { return width; }
        unsigned int getHeight() const { return height; }

        void set(unsigned int width, unsigned int height);
    };
}

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
#include "../basic_types.h"
#include "../geometry.h"


namespace Beatmup {
    /**
        Represents image size in pixels.
    */
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

        /**
            \return `true` if the image width is greater than the height.
        */
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

        operator IntPoint() const { return IntPoint(width, height); }
    };
}

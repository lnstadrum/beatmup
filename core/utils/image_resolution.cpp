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

#include "image_resolution.h"

using namespace Beatmup;

ImageResolution::ImageResolution() : width(0), height(0) {}

ImageResolution::ImageResolution(unsigned int width, unsigned int height) : width(width), height(height) {}

bool ImageResolution::operator == (const ImageResolution& obj) const {
    return (obj.width == width) && (obj.height == height);
}

bool ImageResolution::operator != (const ImageResolution& obj) const {
    return !(*this == obj);
}


msize ImageResolution::numPixels() const {
    return width*height;
}

float ImageResolution::megaPixels() const {
    return width*height / 1000000.0f;
}

float ImageResolution::getAspectRatio() const {
    return (float)width / height;
}

float ImageResolution::getInvAspectRatio() const {
    return (float)height / width;
}

bool ImageResolution::fat() const {
    return width >= height;
}

void ImageResolution::set(unsigned int width, unsigned int height) {
    this->width = width;
    this->height = height;
}

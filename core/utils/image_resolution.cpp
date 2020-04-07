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

IntRectangle ImageResolution::rectangle() const {
    return IntRectangle(0, 0, width - 1, height - 1);
}


void ImageResolution::set(unsigned int width, unsigned int height) {
    this->width = width;
    this->height = height;
}
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

#include "flood_fill.h"
#include "region_filling.h"
#include "../bitmap/processing.h"
#include "../bitmap/internal_bitmap.h"
#include <cstring>

using namespace Beatmup;


FloodFill::FloodFill():
    input(nullptr), output(nullptr), maskPos(0, 0), bounds(0,0,0,0), borderMorphology(NONE), tolerance(0), borderHold(0), borderRelease(0), computeContours(false)
{}


FloodFill::~FloodFill() {
    // deleting contours, if any
    for (IntegerContour2D* contour : contours)
        delete contour;
    contours.clear();
}


void FloodFill::setInput(AbstractBitmap* inputBitmap) {
    this->input = inputBitmap;
}


void FloodFill::setOutput(AbstractBitmap* mask) {
    this->output = mask;
}


void FloodFill::setMaskPos(const IntPoint& pos) {
    this->maskPos = pos;
}


void FloodFill::setSeeds(const IntPoint seeds[], int seedCount) {
    this->seeds.clear();
    for (int n = 0; n < seedCount; n++)
        this->seeds.push_back(seeds[n]);
}


void FloodFill::setSeeds(const int seedsXY[], int seedCount) {
    this->seeds.clear();
    for (int n = 0; n < seedCount; n++)
        this->seeds.push_back(IntPoint(seedsXY[2*n], seedsXY[2*n+1]));
}


void FloodFill::setComputeContours(bool compute) {
    this->computeContours = compute;
}


ThreadIndex FloodFill::getMaxThreads() const {
    return validThreadCount((int)seeds.size());
}


bool FloodFill::process(TaskThread& thread) {
    std::vector<IntPoint> border;
    border.reserve((output->getWidth() + output->getHeight()) * 2);

    std::vector<IntegerContour2D*> myContours;
    IntRectangle bounds;
    bounds.a = bounds.b = seeds[thread.currentThread()];

    for (int n = thread.currentThread(); n < seeds.size(); n += thread.numThreads())
        if (!thread.isTaskAborted()) {
            IntPoint seed = seeds[n];
            if (seed.x >= maskPos.x && seed.y >= maskPos.y && seed.x < maskPos.x + output->getWidth() && seed.y < maskPos.x + output->getHeight()) {
                BitmapProcessing::pipelineWithMaskOutput<Kernels::FillRegion>(*input, *output, maskPos, seed, tolerance, border, bounds);
            }
        }

    // compute contours
    if (!thread.isTaskAborted())
        if (computeContours) {
            BinaryMaskWriter writer(*ignoredSeeds);
            IntegerContour2D::computeBoundary(myContours, *output, border, writer, 0);
        }

    if (!thread.isTaskAborted())
        switch (borderMorphology) {
        case DILATE:
            BitmapProcessing::writeToMask<Kernels::CircularDilatation>(*output, border, 255, borderHold, borderRelease);
            bounds.grow((int) ceilf(borderHold + borderRelease));
            bounds.limit(output->getSize().closedRectangle());
            break;
        case ERODE:
            BitmapProcessing::writeToMask<Kernels::CircularErosion>(*output, border, 255, borderHold, borderRelease);
            bounds.grow(-(int) floorf(borderHold));
            break;
        default: break;
        }

    // updating bounds and merging contours
    {
        std::lock_guard<std::mutex> lock(access);
        if (bounds.a.x < this->bounds.a.x) this->bounds.a.x = bounds.a.x;
        if (bounds.a.y < this->bounds.a.y) this->bounds.a.y = bounds.a.y;
        if (bounds.b.x > this->bounds.b.x) this->bounds.b.x = bounds.b.x;
        if (bounds.b.y > this->bounds.b.y) this->bounds.b.y = bounds.b.y;

        if (computeContours) {
            contours.reserve(contours.size() + myContours.size());
            for (auto contour : myContours)
                contours.push_back(contour);
        }
    }

    return true;
}


void FloodFill::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    NullTaskInput::check(input, "input bitmap");
    NullTaskInput::check(output, "output bitmap");
    lock<ProcessingTarget::CPU>(gpu, input, output);
    for (IntegerContour2D* contour : contours)
        delete contour;
    contours.clear();
    if (computeContours) {
        ignoredSeeds = new InternalBitmap(input->getContext(), PixelFormat::BinaryMask, output->getWidth(), output->getHeight());
        writeLock(gpu, ignoredSeeds,  ProcessingTarget::CPU);
        std::memset(ignoredSeeds->getData(0, 0), 0, ignoredSeeds->getMemorySize());
    }
    bounds.a = bounds.b = seeds[0];
}


void FloodFill::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    unlock(input, output);
    if (computeContours) {
        unlock(ignoredSeeds);
        delete ignoredSeeds;
    }
}


void FloodFill::setTolerance(float tolerance) {
    this->tolerance = tolerance;
}


void FloodFill::setBorderPostprocessing(BorderMorphology operation, float holdRadius, float releaseRadius) {
    borderMorphology = operation;
    borderHold = std::max(0.0f, holdRadius);
    borderRelease = std::max(borderHold, releaseRadius);
}


const IntegerContour2D& FloodFill::getContour(int contourIndex) const {
    RuntimeError::check(contourIndex >= 0 && contourIndex < contours.size(),
        "Contour index is out of range. Contours computation may have been disabled.");
    return *contours[contourIndex];
}

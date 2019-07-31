#include "flood_fill.h"
#include "region_filling.h"
#include "../bitmap/processing.h"
#include "../bitmap/internal_bitmap.h"

using namespace Beatmup;


FloodFill::FloodFill():
	input(NULL), output(NULL), borderMorphology(NONE), maskPos(0, 0), tolerance(0), borderHold(0), borderRelease(0), computeContours(false), bounds(0,0,0,0)
{}


FloodFill::~FloodFill() {
	// deleting contours, if any
	for (IntegerContour2D* contour : contours)
		delete contour;
	contours.clear();
}


void FloodFill::setInput(AbstractBitmap& inputBitmap) {
	this->input = &inputBitmap;
}


void FloodFill::setOutput(AbstractBitmap& mask) {
	this->output = &mask;
}


void FloodFill::setMaskPos(IntPoint& pos) {
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


void FloodFill::setComputeContours(bool computeOrNotCompute) {
	computeContours = computeOrNotCompute;
}


ThreadIndex FloodFill::maxAllowedThreads() const {
	return validThreadCount((int)seeds.size());
}


bool FloodFill::process(TaskThread& thread) {
	std::vector<IntPoint> border;
	border.reserve((output->getWidth() + output->getHeight()) * 2);
	
	std::vector<IntegerContour2D*> myContours;
	IntRectangle bounds;
	bounds.A = bounds.B = seeds[thread.currentThread()];

	for (int n = thread.currentThread(); n < seeds.size(); n += thread.totalThreads())
		if (!thread.isTaskAborted()) {
			IntPoint seed = seeds[n];
			if (seed.x >= maskPos.x && seed.y >= maskPos.y && seed.x < maskPos.x + output->getWidth() && seed.y < maskPos.x + output->getHeight()) {
				BitmapProcessing::pipelineWithMaskOutput<FillRegion>(*input, *output, maskPos.x, maskPos.y, maskPos, seed, tolerance, border, bounds);
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
			BitmapProcessing::writeToMask<CircularDilatation>(*output, 0, 0, border, 255, borderHold, borderRelease);
			bounds.grow((int) ceilf(borderHold + borderRelease));
			bounds.limit(output->getSize().clientRect());
			break;
		case ERODE:
			BitmapProcessing::writeToMask<CircularErosion>(*output, 0, 0, border, 255, borderHold, borderRelease);
			bounds.grow(-(int) floorf(borderHold));
			break;
		}

	// updating bounds and merging contours
	{
		std::lock_guard<std::mutex> lock(access);
		if (bounds.A.x < this->bounds.A.x) this->bounds.A.x = bounds.A.x;
		if (bounds.A.y < this->bounds.A.y) this->bounds.A.y = bounds.A.y;
		if (bounds.B.x > this->bounds.B.x) this->bounds.B.x = bounds.B.x;
		if (bounds.B.y > this->bounds.B.y) this->bounds.B.y = bounds.B.y;

		if (computeContours) {
			contours.reserve(contours.size() + myContours.size());
			for (auto contour : myContours)
				contours.push_back(contour);
		}
	}

	return true;
}


void FloodFill::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
	NullTaskInput::check(input, "input bitmap");
	NullTaskInput::check(output, "output bitmap");
	input->lockPixels(ProcessingTarget::CPU);
	output->lockPixels(ProcessingTarget::CPU);
	for (IntegerContour2D* contour : contours)
		delete contour;
	contours.clear();
	if (computeContours) {
		ignoredSeeds = new InternalBitmap(input->getEnvironment(), PixelFormat::BinaryMask, output->getWidth(), output->getHeight());
		ignoredSeeds->zero();
		ignoredSeeds->lockPixels(ProcessingTarget::CPU);
	}
	bounds.A = bounds.B = seeds[0];
}


void FloodFill::afterProcessing(ThreadIndex threadCount, bool aborted) {
	input->unlockPixels();
	output->unlockPixels();
	if (computeContours) {
		ignoredSeeds->unlockPixels();
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
	if (contourIndex < 0 || contours.size() <= contourIndex)
		BEATMUP_ERROR("Bad contour index, or the contours computation was not enabled");
	return *contours[contourIndex];
}
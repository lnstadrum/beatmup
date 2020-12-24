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
#include "../geometry.h"
#include "../parallelism.h"
#include "../bitmap/abstract_bitmap.h"
#include "../contours/contours.h"
#include <vector>

namespace Beatmup {

    /**
        Flood fill algorithm implementation.
        Discovers areas of similar colors up to a tolerance threshold around given positions (seeds) in the input image.
        These areas are filled with white color in another image (output). If the output bitmap is a binary mask,
        corresponding pixels are set to `1`. The rest of the output image remains unchanged.
        Optionally, computes contours around the discovered areas and stores the contour positions.
        Also optionally, applies post-processing by dilating or eroding the discovered regions in the output image.
    */
    class FloodFill : public AbstractTask, private BitmapContentLock {
    public:
        /**
            Morphological postprocessing operation applied to discovered connected components
        */
        enum BorderMorphology {
            NONE = 0,			//!< no postprocessing
            DILATE,				//!< apply a dilatation
            ERODE				//!< apply an erosion
        };

    protected:
        AbstractBitmap
            *input,									//!< input bitmap
            *output,								//!< resulting mask
            *ignoredSeeds;							//!< 1-bit bitmap storing flags marking used pixels

        IntPoint maskPos;							//!< left-top corner of the mask to compute over the input bitmap
        IntRectangle bounds;						//!< mask bounds
        std::mutex access;							//!< access control assuring internal thread safety
        std::vector<IntPoint> seeds;				//!< set of starting points (seed points)
        std::vector<IntegerContour2D*> contours;	//!< contours to store borders for each seed
        BorderMorphology borderMorphology;			//!< border morphological postprocessing

        float tolerance;							//!< intensity tolerance value
        float
            borderHold,
            borderRelease;
        bool computeContours;					//!< if `true`, border contours will be computed per each seed

    public:
        FloodFill();
        ~FloodFill();

        /**
            Returns input bitmap (null if not set yet)
        */
        const AbstractBitmap* getInput() const { return input; }

        /**
            Returns output bitmap (null if not set yet)
        */
        const AbstractBitmap* getOutput() const { return output; }

        /**
            Returns bounding box of the computed mask
        */
        IntRectangle getBounds() const { return bounds; }

        /**
            Returns number of detected contours
        */
        int getContourCount() const { return contours.size(); }

        /**
            Returns a contour by index if computeContours was `true`, throws an exception otherwise
        */
        const IntegerContour2D& getContour(int contourIndex) const;

        /**
            Sets the input bitmap
        */
        void setInput(AbstractBitmap*);

        /**
            Specifies the bitmap to put the resulting mask to
        */
        void setOutput(AbstractBitmap*);

        /**
            Specifies left-top corner position of the mask to compute inside the input bitmap
        */
        void setMaskPos(const IntPoint&);

        /**
            Specifies a set of seeds (starting points)
        */
        void setSeeds(const IntPoint seeds[], int seedCount);
        void setSeeds(const int seedsXY[], int seedCount);

        /**
            Sets the intensity tolerance threshold used to decide on similarity of neighboring pixels.
        */
        void setTolerance(float);

        /**
            Returns yjr intensity tolerance threshold.
        */
        inline float getTolerance() const { return tolerance; };

        /**
            Specifies a morphological operation to apply to the mask border.
            \param operation          A postprocessing operation
            \param holdRadius         Erosion/dilation hold radius (output values set to 1)
            \param releaseRadius      Erosion/dilation radius of transition from 1 to 0
        */
        void setBorderPostprocessing(BorderMorphology operation, float holdRadius, float releaseRadius);

        /**
            Enables or disables contours computation.
        */
        void setComputeContours(bool);

        ThreadIndex getMaxThreads() const;
        virtual bool process(TaskThread& thread) final;
        virtual void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) final;
        virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) final;
    };

}
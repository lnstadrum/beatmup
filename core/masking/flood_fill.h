/*
    Flood fill
*/
#pragma once
#include "../geometry.h"
#include "../parallelism.h"
#include "../bitmap/abstract_bitmap.h"
#include "../contours/contours.h"
#include <vector>
namespace Beatmup {
    class FloodFill : public AbstractTask {
    public:
        //!< border morphological postprocessing
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
        IntPoint maskPos;							//!< left-top corner of the mask over the input bitmap
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
            Returns input bitmap (NULL if not set yet)
        */
        AbstractBitmap* getInput() const { return input; }
        /**
            Returns output bitmap (NULL if not set yet)
        */
        AbstractBitmap* getOutput() const { return output; }
        /**
            \returns bounding rect of the computed mask
        */
        IntRectangle getBounds() const { return bounds; }
        /**
            \return number of detected contours
        */
        int getContourCount() const { return contours.size(); }
        /**
            \return a contour by index if computeContours was `true`, throws an exception otherwise
        */
        const IntegerContour2D& getContour(int contourIndex) const;
        /**
            Sets the input bitmap
        */
        void setInput(AbstractBitmap&);
        /**
            Specifies the bitmap to put the resulting mask to
        */
        void setOutput(AbstractBitmap&);
        /**
            Specifies left-top corner position of the mask inside the input bitmap
        */
        void setMaskPos(const IntPoint&);
        /**
            Specifies a set of seeds (starting points)
        */
        void setSeeds(const IntPoint seeds[], int seedCount);
        void setSeeds(const int seedsXY[], int seedCount);
        /**
            Specifies intensity tolerance value
        */
        void setTolerance(float tolerance);
        /**
            Specifies a morphological operation to apply to the mask border
        */
        void setBorderPostprocessing(BorderMorphology operation, float holdRadius, float releaseRadius);
        /**
            Enables or disables contours computation
        */
        void setComputeContours(bool);
        ThreadIndex maxAllowedThreads() const;
        virtual bool process(TaskThread& thread) final;
        virtual void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) final;
        virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) final;
    };}
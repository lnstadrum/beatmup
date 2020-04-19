/*
    Contour processing
*/

#pragma once
#include "../geometry.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/mask_bitmap_access.h"
#include "../exception.h"
#include <vector>
#include <mutex>

namespace Beatmup {

    /**
        A sequence of integer-valued 2D points
    */
    class IntegerContour2D {
    private:
        std::vector<IntPoint> points;
        float
            totalLength,
            lastFragmentLength2;
    public:
        IntegerContour2D();

        /**
            Adds a new point to the contour. Some points may be skipped to optimize the storage.
            \param x	new point X coordinate
            \param y	new point Y coordinate
        */
        void addPoint(int x, int y);


        /**
            Removes contour content
        */
        void clear();


        /**
            \return available point count
        */
        int getPointCount() const { return (int)points.size(); };


        float getLenght() const { return totalLength; };


        /**
            \return a point by its index
        */
        inline IntPoint getPoint(int index) const { return points[index % points.size()]; };

        /**
            Discovers an area boundary in a bitmap following a level curve, starting from a given set of points
            \param boundary			vector to put connected components of the detected boundary to
            \param bitmap			the bitmap to discover
            \param border			the starting points
            \param testedPixels		a writer of a binary mask bitmap marking pixels that are already processed
            \param level			the level of the curve
            \returns a vector of all the contours limiting the area
        */
        static void computeBoundary(
            std::vector<IntegerContour2D*>& boundary,
            AbstractBitmap& bitmap,
            std::vector<IntPoint>& border,
            BinaryMaskWriter& testedPixels,
            float level = 0.5f
        );


        class BadSeedPoint : public Exception {
        public:
            BadSeedPoint() : Exception("Bad seeds: no border points found")
            {}

            BadSeedPoint(int x, int y, bool lefttop, bool righttop, bool leftbottom, bool rightbottom):
                Exception("Bad seed (%d,%d), pattern:\n  %d%d\n  %d%d\n", x, y, lefttop, righttop, leftbottom, rightbottom)
            {}
        };
    };
}

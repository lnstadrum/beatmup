#include "contours.h"
#include "../bitmap/processing.h"
#include <algorithm>
#include <cstdlib>		// because of std::abs stuff on integer numbers

using namespace Beatmup;


template<class in_t> class ComputeBounds {
public:
	static void process(in_t in, std::vector<IntegerContour2D*>& boundary, std::vector<IntPoint>& border, BinaryMaskWriter& testedPixels, float level) {
		typedef struct {
			int x, y;
		} Displacement;

		// clockwise direction lookup in function of four neighbour values
		const Displacement LOOKUP_DIRECTION[2][2][2][2] = {
			//rb			//lb		//rb+lb			//rt		//rt+rb			//rt+lb		//rt+lb+rb
			{ { { { 0, 0 }, { 1, 0 } }, { { 0, +1 }, { +1, 0 } } }, { { { 0, -1 }, { 0, -1 } }, { { 9, 9 }, { 0, -1 } } } },
			//lt	//lt+rb			//lt+lb		//lt+lb+rb		//lt+rt		//lt+rt+rb		//lt+rt+lb
			{ { { { -1, 0 }, { 9, 9 } }, { { 0, 1 }, { +1, 0 } } }, { { { -1, 0 }, { -1, 0 } }, { { 0, 1 }, { 0, 0 } } } }
		};

		const Displacement LOOKUP_INTERNAL_PIXEL[3][3] = {
			{ { 0, 0 }, { 0, -1 }, { 0, 0 } },
			{ { -1, -1 }, { 9, 0 }, { 0, 0 } },
			{ { 0, 0 }, { -1, 0 }, { 0, 0 } }
		};

		const int W = in.getWidth(), H = in.getHeight();
		const float LEVEL = level * in.MAX_VALUE;
		IntPoint seed;
		int x, y;

		// scanning boundary pixels
		for (auto seed : border) {

			// picking a seed
			x = seed.x;
			y = seed.y;
			testedPixels.goTo(x, y);
			if (testedPixels.getValue() > 0)
				continue;
			testedPixels.putValue(1);

			// checking if the seed is good
			bool
				rb = x < W && y < H && in().mean() > LEVEL,
				lb = x > 0 && y < H && in(x - 1, y).mean() > LEVEL,
				rt = x < W && y > 0 && in(x, y - 1).mean() > LEVEL,
				lt = x > 0 && y > 0 && in(x - 1, y - 1).mean() > LEVEL,
				//goodSeedFound = (!(rb && lb && rt && lt) && (rb || lb || rt || lt));
				goodSeedFound = (rb && !(lb && rt && lt));

			if (!goodSeedFound)
				continue;

			// good seed found: go
			in.goTo(x, y);
			Displacement prev = { 0, 0 };
			IntegerContour2D* contour = new IntegerContour2D();
			boundary.push_back(contour);

			// go
			do {
				// binarize the neighbours
				bool
					rb = x < W && y < H && in().mean() > LEVEL,
					lb = x > 0 && y < H && in(x - 1, y).mean() > LEVEL,
					rt = x < W && y > 0 && in(x, y - 1).mean() > LEVEL,
					lt = x > 0 && y > 0 && in(x - 1, y - 1).mean() > LEVEL;

				// check the point: must have a neighbor inside and a neighbor outside
				if ((rb && lb && rt && lt) || !(rb || lb || rt || lt))
					throw IntegerContour2D::BadSeedPoint(x, y, lt, rt, lb, rb);

				// add the point
				contour->addPoint(x, y);

				// compute step direction	
				Displacement d = LOOKUP_DIRECTION[lt][rt][lb][rb];
				if (d.x == 9) {		// special case: displacement depends on the previous direction
					if (prev.x == 0 && prev.y == 0)
						throw IntegerContour2D::BadSeedPoint(x, y, lt, rt, lb, rb);
					d.x = -prev.y;
					d.y = prev.x;
				}

				// marking used pixel
				Displacement ip = LOOKUP_INTERNAL_PIXEL[d.y + 1][d.x + 1];
				int ipx = x + ip.x, ipy = y + ip.y;
				if (ipx >= 0 && ipy >= 0 && ipx < W && ipy < H) {
					testedPixels.goTo(ipx, ipy);
					testedPixels.putValue(1);
				}

				// step
				x += d.x;
				y += d.y;
				prev = d;

				if (x < W && y < H)
					in.goTo(x, y);

			} while (seed.x != x || seed.y != y);

			// adding the last point
			contour->addPoint(x, y);
		}
	}
};


IntegerContour2D::IntegerContour2D() :
	totalLength(0.0f), lastFragmentLength2(0.0f)
{}


void IntegerContour2D::addPoint(int x, int y) {
	// check for update
	if (points.size() > 2) {
		IntPoint& p1 = points.back();
		IntPoint p2 = points[points.size() - 2];
		if (
			(std::max(abs(x - p1.x), abs(y - p1.y)) <= 1 && std::max(abs(x - p2.x), abs(y - p2.y)) <= 1) ||
			(x != p1.x  &&  (p1.y - y) * (p2.x - p1.x) == (p2.y - p1.y) * (p1.x - x)) ||
			(y != p1.y  &&  (p1.x - x) * (p2.y - p1.y) == (p2.x - p1.x) * (p1.y - y))
		) {
			// update the last point in this case, not to add a new one
			p1.x = x;
			p1.y = y;
			lastFragmentLength2 = (float)(p1 - p2).hypot2();
			return;
		}
		if (x == p1.x && y == p1.y)
			return;			//p1 == (x,y)
	}

	// adding the new point
	IntPoint p(x, y);
	points.push_back(p);
	totalLength += sqrt(lastFragmentLength2);
	lastFragmentLength2 = 0;
}


void IntegerContour2D::computeBoundary(std::vector<IntegerContour2D*>& boundary, AbstractBitmap& bitmap, std::vector<IntPoint>& border, BinaryMaskWriter& testedPixels, float level) {
	BitmapProcessing::read<ComputeBounds>(
		bitmap, border[0].x, border[0].y,
		boundary, border, testedPixels, level
	);
}


void IntegerContour2D::clear() {
	points.clear();
	totalLength = lastFragmentLength2 = 0;
}
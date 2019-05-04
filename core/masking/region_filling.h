/*

*/
#pragma once
#include "../geometry.h"
#include <queue>
#include <mutex>

namespace Beatmup {

	template<typename in_t, typename out_t> class FillRegion {
	public:
		typedef typename in_t::pixtype::operating_type inpixvaltype;
		/**
			Region filling
			\param in			input bitmap reader
			\param out			output mask writer
			\param maskOffset	mask position in the bitmap
			\param seed			entry point
			\param tolerance	tolerance level: how much a pixel has to be different from seed to not to be filled
			\param border		a vector to put border points to for further processing
		*/
		static void process(
			in_t in,
			out_t out,
			IntPoint maskOffset,
			IntPoint seed,
			inpixvaltype tolerance,
			std::vector<IntPoint>& border,
			IntRectangle& bounds
		) {
			const int
				W = in.getWidth() - 1, H = in.getHeight() - 1,
				MW = out.getWidth() - 1, MH = out.getHeight() - 1;

			std::queue<IntPoint> queue;
			queue.push(IntPoint(seed.x, seed.y));
			in.goTo(seed.x, seed.y);
			
			const typename in_t::pixtype ref = in();	// reference input value
			const int range = out.MAX_UNNORM_VALUE;

			inpixvaltype diff;
			unsigned char newval, oldval;

			// performance critical cycle
			while (!queue.empty()) {
				// popping from queue front
				IntPoint p = queue.front();
				queue.pop();
				int
					mx = p.x - maskOffset.x,
					my = p.y - maskOffset.y;

				// checking neighbor pixels
				bool onBorder = false;

				if (p.x > 0 && mx > 0 && (diff = (in(p.x - 1, p.y) - ref).abs().max()) <= tolerance) {
					newval = (tolerance > 0 && diff > 0) ? (range * (tolerance - diff) / tolerance + 1) : out.MAX_UNNORM_VALUE;
					out.goTo(mx - 1, my);
					oldval = out.getValue();
					if (oldval < newval) {
						out.putValue(newval);
						queue.push(IntPoint(p.x - 1, p.y));
					}
				}
				else onBorder = true;

				if (p.y > 0 && my > 0 && (diff = (in(p.x, p.y - 1) - ref).abs().max()) <= tolerance) {
					newval = (tolerance > 0 && diff > 0) ? (range * (tolerance - diff) / tolerance + 1) : out.MAX_UNNORM_VALUE;
					out.goTo(mx, my - 1);
					oldval = out.getValue();
					if (oldval < newval) {
						out.putValue(newval);
						queue.push(IntPoint(p.x, p.y - 1));
					}
				}
				else onBorder = true;

				if (p.x < W && mx < MW && (diff = (in(p.x + 1, p.y) - ref).abs().max()) <= tolerance) {
					newval = (tolerance > 0 && diff > 0) ? (range * (tolerance - diff) / tolerance + 1) : out.MAX_UNNORM_VALUE;
					out.goTo(mx + 1, my);
					oldval = out.getValue();
					if (oldval < newval) {
						out.putValue(newval);
						queue.push(IntPoint(p.x + 1, p.y));
					}
				}
				else onBorder = true;

				if (p.y < H && my < MH && (diff = (in(p.x, p.y + 1) - ref).abs().max()) <= tolerance) {
					newval = (tolerance > 0 && diff > 0) ? (range * (tolerance - diff) / tolerance + 1) : out.MAX_UNNORM_VALUE;
					out.goTo(mx, my + 1);
					oldval = out.getValue();
					if (oldval < newval) {
						out.putValue(newval);
						queue.push(IntPoint(p.x, p.y + 1));
					}
				}
				else onBorder = true;

				if (onBorder) {
					border.push_back(IntPoint(mx, my));

					if (mx < bounds.A.x)
						bounds.A.x = mx;
					if (mx > bounds.B.x)
						bounds.B.x = mx;
					if (my < bounds.A.y)
						bounds.A.y = my;
					if (my > bounds.B.y)
						bounds.B.y = my;
				}
			}
		}
	};



	template<typename out_t> class CircularDilatation {
	public:
		/**
			Circular dilatation of a mask at given points
			\param mask			the mask
			\param pointSet		the points
			\param val			max value (amplitude)
			\param holdRad		inner kernel radius; all pixels inside take `val` value
			\param releaseRad	release ring outer radius; all pixels in the ring take linearly attenuated `val` value
		*/
		static void process(out_t mask, std::vector<IntPoint>& pointSet, int val, float holdRad, float releaseRad) {
			const int morphoSize = (int)ceilf(holdRad + releaseRad);
			const float morphoReleaseRing = releaseRad - holdRad;
			// for each point in the point set...
			for (auto p : pointSet) {
				// determine a bounding box to apply the kernel
				int
					x1 = std::max(0, p.x - morphoSize),
					y1 = std::max(0, p.y - morphoSize),
					x2 = std::min(mask.getWidth() - 1, p.x + morphoSize),
					y2 = std::min(mask.getHeight() - 1, p.y + morphoSize);
				// apply the kernel
				for (int y = y1; y <= y2; ++y) {
					mask.goTo(x1, y);
					for (int x = x1; x <= x2; ++x, mask++) {
						// squared distance to center
						int d2 = sqr(x - p.x) + sqr(y - p.y);
						if (d2 < sqr(holdRad))
							mask.assign(val);
						else if (d2 < sqr(releaseRad)) {
							// linear attenuation
							int c = (int)roundf((float)val * (1.0f - (sqrtf((float)d2) - holdRad) / morphoReleaseRing));
							if (mask().x < c)
								mask.assign(c);
						}
					}
				}
			}
		}
	};


	template<typename out_t> class CircularErosion {
	public:
		/**
			Circular erosion of a mask at given points
			\param mask			the mask
			\param pointSet		the points
			\param val			max value (amplitude)
			\param holdRad		inner kernel radius; all pixels inside take `val` value
			\param releaseRad	release ring outer radius; all pixels in the ring take linearly attenuated `val` value
		*/
		static void process(out_t mask, std::vector<IntPoint>& pointSet, int val, float holdRad, float releaseRad) {
			const int morphoSize = (int)ceilf(holdRad + releaseRad);
			const float morphoReleaseRing = releaseRad - holdRad;
			// for each point in the point set...
			for (auto p : pointSet) {
				// determine a bounding box to apply the kernel
				int
					x1 = std::max(0, p.x - morphoSize),
					y1 = std::max(0, p.y - morphoSize),
					x2 = std::min(mask.getWidth() - 1, p.x + morphoSize),
					y2 = std::min(mask.getHeight() - 1, p.y + morphoSize);
				// apply the kernel
				for (int y = y1; y <= y2; ++y) {
					mask.goTo(x1, y);
					for (int x = x1; x <= x2; ++x, mask++) {
						// squared distance to center
						int d2 = sqr(x - p.x) + sqr(y - p.y);
						if (d2 < sqr(holdRad))
							mask.assign(0);
						else if (d2 < sqr(releaseRad)) {
							// linear attenuation
							int c = (int)roundf((float)val * (sqrtf((float)d2) - holdRad) / morphoReleaseRing);
							if (mask().x > c)
								mask.assign(c);
						}
					}
				}
			}
		}
	};
}
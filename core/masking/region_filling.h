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
#include "../bitmap/abstract_bitmap.h"
#include "../geometry.h"
#include <queue>
#include <mutex>

using namespace Beatmup;

namespace Kernels {
    /**
        Region filling kernel implementing flood fill starting from a given seed
    */
    template<typename in_t, typename out_t> class FillRegion {
    public:
        typedef typename in_t::pixtype::operating_type inpixvaltype;
        /**
            Fills a region in an output bitmap starting from a given position in an input bitmap
            \param input        Input bitmap reader
            \param output       Output mask writer
            \param maskOffset   Mask position in the bitmap
            \param seed         Entry point
            \param tolerance    Tolerance level: how much a pixel has to be different from seed to not to be filled
            \param border       A vector to put border points to for further processing
            \param bounds       Bounding box of the filled region; the input value is updated but not reset
        */
        static void process(
            AbstractBitmap& input,
            AbstractBitmap& output,
            IntPoint maskOffset,
            IntPoint seed,
            inpixvaltype tolerance,
            std::vector<IntPoint>& border,
            IntRectangle& bounds
        ) {
            in_t in(input);
            out_t out(output);

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

                    if (mx < bounds.a.x)
                        bounds.a.x = mx;
                    if (mx > bounds.b.x)
                        bounds.b.x = mx;
                    if (my < bounds.a.y)
                        bounds.a.y = my;
                    if (my > bounds.b.y)
                        bounds.b.y = my;
                }
            }
        }
    };


    /**
        Circular dilatation kernel for flood fill contours postprocessing
    */
    template<typename out_t> class CircularDilatation {
    public:
        /**
            Circular dilatation of a mask at given points
            \param bitmap       The mask bitmap
            \param pointSet     The points
            \param val          Max value (amplitude)
            \param holdRad      Inner kernel radius; all pixels inside take `val` value
            \param releaseRad   Release ring outer radius; all pixels in the ring take linearly attenuated `val` value
        */
        static void process(AbstractBitmap& bitmap, std::vector<IntPoint>& pointSet, int val, float holdRad, float releaseRad) {
            out_t mask(bitmap);
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


    /**
        Circular erosion kernel for flood fill contours postprocessing
    */
    template<typename out_t> class CircularErosion {
    public:
        /**
            Circular erosion of a mask at given points
            \param bitmap       The mask bitmap
            \param pointSet     The points
            \param val          Max value (amplitude)
            \param holdRad      Inner kernel radius; all pixels inside take `val` value
            \param releaseRad   Release ring outer radius; all pixels in the ring take linearly attenuated `val` value
        */
        static void process(AbstractBitmap& bitmap, std::vector<IntPoint>& pointSet, int val, float holdRad, float releaseRad) {
            out_t mask(bitmap);
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

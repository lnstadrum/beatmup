/*
    Bitmap resampling utilities
*/

#pragma once
#include "../parallelism.h"
#include "../geometry.h"
#include <algorithm>

namespace Beatmup {
namespace BitmapResamplingTools{


    template<class in_t, class out_t> class NearestNeigborResampling {
    public:

        /**
            Resamples a rectangle from an input bitmap to a rectangle in an output bitmap by nearest neighbor interpolation
        */
        static void process(in_t in, out_t out, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            const int
                srcW = src.width(), srcH = src.height(),
                dstW = dst.width(), dstH = dst.height(),
                shiftX = srcW / 2,
                shiftY = srcH / 2;

            // dest image slice to process in the current thread
            const int
              sliceStart = tt.currentThread()       * dstH / tt.totalThreads(),
              sliceStop  = (tt.currentThread() + 1) * dstH / tt.totalThreads();

            for (int y = sliceStart; y < sliceStop; ++y) {
                out.goTo(dst.a.x, dst.a.y + y);
                const int sy = src.a.y + (y * srcH + shiftY) / dstH;

                for (int x = 0; x < dstW; ++x) {
                    const int sx = src.a.x + (x * srcW + shiftX) / dstW;
                    in.goTo(sx, sy);
                    out = in();
                    out++;
                }

                if (tt.isTaskAborted())
                    return;
            }
        }
    };


    template<class in_t, class out_t> class BoxResampling {
    public:

        /**
            Resamples a rectangle from an input bitmap to a rectangle in an output bitmap applying a box filter
        */
        static void process(in_t in, out_t out, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            const int
                srcW = src.width(), srcH = src.height(),
                dstW = dst.width(), dstH = dst.height();

            // dest image slice to process in the current thread
            const int
              sliceStart = tt.currentThread()       * dstH / tt.totalThreads(),
              sliceStop  = (tt.currentThread() + 1) * dstH / tt.totalThreads();

            int x0, y0, x1, y1 = src.a.y + (sliceStart) * srcH / dstH;    // coordinates of source pixels box mapped to a given dest pixel

            typename in_t::pixtype acc;
            for (int y = sliceStart; y < sliceStop; ++y) {
                out.goTo(dst.a.x, dst.a.y + y);
                y0 = y1;
                y1 = src.a.y + (y + 1) * srcH / dstH;
                x1 = src.a.x;
                for (int x = 0; x < dstW; ++x) {
                    x0 = x1;
                    x1 = src.a.x + (x + 1) * srcW / dstW;

                    // loop over the source area
                    acc.zero();
                    int y = y0;
                    do {
                        in.goTo(x0, y);
                        int x = x0;
                        do {
                            acc = acc + in();
                            in++;
                        } while (++x < x1);
                    } while (++y < y1);

                    // write out
                    out = acc / std::max(1, (x1 - x0)*(y1 - y0));
                    out++;
                }

                if (tt.isTaskAborted())
                    return;
            }
        }
    };


    template<class in_t, class out_t> class BilinearResampling {
    public:

        /**
            Resamples a rectangle from an input bitmap to a rectangle in an output bitmap by bilinear interpolation
        */
        static void process(in_t in, out_t out, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            const int
                srcW = src.width(), srcH = src.height(),
                dstW = dst.width(), dstH = dst.height(),
                shiftX = srcW > dstW ? srcW / 2 : srcW < dstW ? -srcW / 2 : 0,
                shiftY = srcH > dstH ? srcH / 2 : srcH < dstH ? -srcH / 2 : 0;

            // dest image slice to process in the current thread
            const int
              sliceStart = tt.currentThread()       * dstH / tt.totalThreads(),
              sliceStop  = (tt.currentThread() + 1) * dstH / tt.totalThreads();

            for (int y = sliceStart; y < sliceStop; ++y) {
                out.goTo(dst.a.x, dst.a.y + y);
                const float fsy = (float)(y * srcH + shiftY) / dstH;
                const int   isy = (int)fsy;
                const float fy = fsy - (float)isy, _fy = 1 - fy;
                const int   sy = src.a.y + isy;

                in.goTo(src.a.x, sy);
                const int
                    lineJump = sy < srcH - 1 ? srcW - 1 : -1,
                    xBound = srcW - 1;

                typename out_t::pixtype acc;
                for (int x = 0; x < dstW; ++x) {

                    const float fsx = (float)(x * srcW + shiftX) / dstW;
                    const int   isx = (int)fsx;
                    const float fx = fsx - (float)isx;
                    const int   sx = src.a.x + isx;

                    in.goTo(sx, sy);
                    if (sx < xBound) {
                        acc = in() * (1 - fx) * _fy;
                        in++;
                        acc = acc + in() * fx * _fy;
                        in += lineJump;
                        acc = acc + in() * (1 - fx) * fy;
                        in++;
                        acc = acc + in() * fx * fy;
                    }
                    else {
                        acc = in() * _fy;
                        in += lineJump + 1;
                        acc = acc + in() * fy;
                    }

                    out = acc;
                    out++;
                }

                if (tt.isTaskAborted())
                    return;
            }
        }
    };

}
}

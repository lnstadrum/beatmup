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
        static void process(AbstractBitmap& input, AbstractBitmap& output, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            in_t in(input);
            out_t out(output);

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
        static void process(AbstractBitmap& input, AbstractBitmap& output, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            in_t in(input);
            out_t out(output);

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
        static void process(AbstractBitmap& input, AbstractBitmap& output, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            in_t in(input);
            out_t out(output);

            const int
                srcW = src.width(), srcH = src.height(),
                dstW = dst.width(), dstH = dst.height(),
                shiftX = (srcW - dstW) / 2,
                shiftY = (srcH - dstH) / 2;

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


    template<class in_t, class out_t> class BicubicResampling {
    private:
        /**
            Precomputes kernel coefficients in function of bicubic kernel parameter.
        */
        class BicubicKernel {
        private:
            float c0[2], c1[2], c2[3];
            float coeff[4];
        public:
            BicubicKernel(const float alpha) {
                // alpha*x**3 - 2*alpha*x**2 + alpha*x
                c0[0] = alpha;
                c0[1] = -2 * alpha;
                //alpha*x**3 - alpha*x**2 + 2*x**3 - 3*x**2 + 1
                c1[0] = - alpha - 3;
                c1[1] = alpha + 2;
                // -alpha*x**3 + 2*alpha*x**2 - alpha*x - 2*x**3 + 3*x**2
                c2[0] = -alpha;
                c2[1] = 2 * alpha + 3;
                c2[2] = - alpha - 2;
            }

            /**
                Computes the kernel itself for a given phase.
            */
            void operator()(float x) {
                const float xx = x * x, xxx = xx * x;
                coeff[0] = c0[0] * (xxx + x) + c0[1] * xx;
                coeff[1] = c1[1] * xxx + c1[0] * xx + 1;
                coeff[2] = c2[2] * xxx + c2[1] * xx + c2[0] * x;
                coeff[3] = 1 - coeff[0] - coeff[1] - coeff[2];
            }

            inline const float operator[](const int i) const {
                return coeff[i];
            }
        };

    public:
        /**
            Resamples a rectangle from an input bitmap to a rectangle in an output bitmap applying a bicubic kernel
        */
        static void process(AbstractBitmap& input, AbstractBitmap& output, IntRectangle& src, IntRectangle& dst, const float alpha, const TaskThread& tt) {
            in_t in(input);
            out_t out(output);

            const int
                srcW = src.width(), srcH = src.height(),
                dstW = dst.width(), dstH = dst.height(),
                shiftX = (srcW - dstW) / 2,
                shiftY = (srcH - dstH) / 2;

            // dest image slice to process in the current thread
            const int
              sliceStart = tt.currentThread()       * dstH / tt.totalThreads(),
              sliceStop  = (tt.currentThread() + 1) * dstH / tt.totalThreads();

            BicubicKernel kx(alpha), ky(alpha);

            for (int y = sliceStart; y < sliceStop; ++y) {
                out.goTo(dst.a.x, dst.a.y + y);
                const float fsy = (float)(y * srcH + shiftY) / dstH;
                const int   isy = (int)fsy;
                const float fy = fsy - (float)isy, _fy = 1 - fy;
                const int   sy = src.a.y + isy;

                const int lineJump[3] = {
                    sy > 0 ? srcW : 0,
                    sy < srcH - 1 ? srcW : 0,
                    sy < srcH - 2 ? srcW : 0
                };

                // preparing kernel
                ky(fy);

                typename out_t::pixtype acc;
                for (int x = 0; x < dstW; ++x) {

                    const float fsx = (float)(x * srcW + shiftX) / dstW;
                    const int   isx = (int)fsx;
                    const float fx = fsx - (float)isx;
                    const int   sx = src.a.x + isx;

                    kx(fx);

                    const int pixJump[3] = {
                        sx > 0        ? -1 : 0,
                        sx < srcW - 1 ? +1 : 0,
                        sx < srcW - 2 ? +2 : 0
                    };

                    in.goTo(sx, sy > 0 ? sy - 1 : 0);
                    acc =       in[pixJump[0]] * kx[0] * ky[0] + in() * kx[1] * ky[0] + in[pixJump[1]] * kx[2] * ky[0] + in[pixJump[2]] * kx[3] * ky[0];
                    in += lineJump[0];
                    acc = acc + in[pixJump[0]] * kx[0] * ky[1] + in() * kx[1] * ky[1] + in[pixJump[1]] * kx[2] * ky[1] + in[pixJump[2]] * kx[3] * ky[1];
                    in += lineJump[1];
                    acc = acc + in[pixJump[0]] * kx[0] * ky[2] + in() * kx[1] * ky[2] + in[pixJump[1]] * kx[2] * ky[2] + in[pixJump[2]] * kx[3] * ky[2];
                    in += lineJump[2];
                    acc = acc + in[pixJump[0]] * kx[0] * ky[3] + in() * kx[1] * ky[3] + in[pixJump[1]] * kx[2] * ky[3] + in[pixJump[2]] * kx[3] * ky[3];

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

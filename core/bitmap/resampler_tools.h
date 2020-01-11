/*
    Bitmap resampling utilities
*/

#pragma once
#include "../parallelism.h"
#include "../geometry.h"
#include <algorithm>

namespace Beatmup {
namespace BitmapResamplingTools{

    template<class in_t, class out_t> class CumulatingResampler {
    public:

        /**
            Resamples a rect from an input bitmap to a rect in an output bitmap
        */
        static void process(in_t in, out_t out, IntRectangle& src, IntRectangle& dst, const TaskThread& tt) {
            const int
                WS = src.width() + 1, HS = src.height() + 1,
                WD = dst.width() + 1, HD = dst.height() + 1;

            typename in_t::pixtype buffa;
            int X0, Y0, X1, Y1 = src.a.y;

            for (int y = tt.currentThread(); y < HD; y += tt.totalThreads()) {
                out.goTo(dst.a.x, dst.a.y + y);
                Y0 = Y1;
                Y1 = src.a.y + (y + 1) * HS / HD;
                X1 = src.a.x;
                for (int x = 0; x < WD; ++x) {
                    X0 = X1;
                    X1 = src.a.x + (x + 1) * WS / WD;
                    // source cycle
                    buffa.zero();
                    int Y = Y0;
                    do {
                        in.goTo(X0, Y);
                        int X = X0;
                        do {
                            buffa = buffa + in();
                            in++;
                        } while (++X < X1);
                    } while (++Y < Y1);
                    out = buffa / std::max(1, ((X1 - X0)*(Y1 - Y0)));
                    out++;
                }

                if (tt.isTaskAborted())
                    return;
            }
        }
    };
}}
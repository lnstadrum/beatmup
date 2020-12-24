/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include "sepia.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/processing.h"
#include <algorithm>

using namespace Beatmup;

namespace Kernels {
    /**
        Application of sepia filter on CPU
    */
    template <class in_t, class out_t> class ApplySepia {
    public:
        static inline void process(AbstractBitmap& input, AbstractBitmap& output, int x, int y, msize nPix) {
            in_t in(input, x, y);
            out_t out(output, x, y);
            static const pixint3
                R{ 100, 196, 48 },
                G{  89, 175, 43 },
                B{  69, 138, 33 };

            for (msize n = 0; n < nPix; n++) {
                pixint4 P = (pixint4)in();
                out.assign(
                    (P.r * R.r + P.g * R.g + P.b * R.b) / 255,
                    (P.r * G.r + P.g * G.g + P.b * G.b) / 255,
                    (P.r * B.r + P.g * B.g + P.b * B.b) / 255,
                    P.a
                );
                in++;
                out++;
            }
        }
    };
}


void Filters::Sepia::apply(int x, int y, msize nPix, TaskThread& thread) {
    BitmapProcessing::pipeline<Kernels::ApplySepia>(*inputBitmap, *outputBitmap, x, y, nPix);
}


std::string Filters::Sepia::getGlslSourceCode() const { 
    return
        "gl_FragColor = vec4(dot(vec3(100.0/255.0, 196.0/255.0, 48.0/255.0), "  + GLSL_RGBA_INPUT + ".rgb)," +
                            "dot(vec3( 89.0/255.0, 175.0/255.0, 43.0/255.0), "  + GLSL_RGBA_INPUT + ".rgb)," +
                            "dot(vec3( 69.0/255.0, 138.0/255.0, 33.0/255.0), "  + GLSL_RGBA_INPUT + ".rgb)," +
                            GLSL_RGBA_INPUT + ".a);";
}
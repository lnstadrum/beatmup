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
#include "../bitmap/pixel_arithmetic.h"
#include <algorithm>
#include <math.h>


namespace Beatmup {
/**
    %Color representation and conversion tools
*/
namespace Color {

    /**
        HSVA quad (for hue, saturation, value and alpha)
    */
    struct hsva_t {
        pixfloat h, s, v;

        /**
            \brief Constructs an HSVA quad from r, g, b, a values
        */
        inline hsva_t(pixfloat r, pixfloat g, pixfloat b) {
            v = std::max(std::max(r, g), b);
            float C = v - std::min(std::min(r, g), b);
            if (C == 0)
                h = 0;
            else if (v == r)
                h = modf((g - b) / C, 6) / 6;
            else if (v == g)
                h = ((b - r) / C + 2) / 6;
            else if (v == b)
                h = ((r - g) / C + 4) / 6;
            s = v > 0 ? C / v : 0;
        }


        /**
            \brief Conversion to pixfloat
        */
        inline operator pixfloat4() const {
            const float
                H = h - (long)h + (h >= 0 ? 0 : 1),
                C = v*s,
                X = C*(1 - fabs(modf(H*6, 2) - 1)),
                m = v - C;

            pixfloat4 out;
            if (H < 1.0f / 6) {
                out.r = C + m; out.g = X + m; out.b = m;
            }
            else if (H < 2.0f / 6) {
                out.r = X + m; out.g = C + m; out.b = m;
            }
            else if (H < 3.0f / 6) {
                out.r = m; out.g = C + m; out.b = X + m;
            }
            else if (H < 4.0f / 6) {
                out.r = m; out.g = X + m; out.b = C + m;
            }
            else if (H < 5.0f / 6) {
                out.r = X + m; out.g = m; out.b = C + m;
            }
            else {
                out.r = C + m; out.g = m; out.b = X + m;
            }
            return out;
        }

        inline hsva_t() : h(0), s(0), v(0) {}

        inline hsva_t(const color3i& _) : hsva_t(_.r, _.g, _.b) {}
        inline hsva_t(const color4i& _) : hsva_t(_.r, _.g, _.b) {}
        inline hsva_t(const color3f& _) : hsva_t((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}
        inline hsva_t(const color4f& _) : hsva_t((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}

        inline hsva_t(const pixfloat1& _) {
            h = s = 0;
            v = _.x;
        }
        
        inline hsva_t(const pixfloat3& _) : hsva_t(_.r, _.g, _.b) {}

        inline hsva_t(const pixfloat4& _) : hsva_t(_.r, _.g, _.b) {}
        
        inline hsva_t(const pixint1& _) {
            h = s = 0;
            v = (float)_.x / 255;
        }

        inline hsva_t(const pixint3& _) : hsva_t((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}

        inline hsva_t(const pixint4& _) : hsva_t((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}

    };

}
}
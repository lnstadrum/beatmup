#pragma once
#include "../bitmap/pixel_arithmetic.h"
#include <algorithm>
#include <math.h>


namespace Beatmup {

	/**
		HSVA quad
	*/
	struct colorhsv {
		pixfloat h, s, v;

		/**
			\brief Constructs an HSVA quad from r, g, b, a values
		*/
		inline colorhsv(pixfloat r, pixfloat g, pixfloat b) {
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

		inline colorhsv() : h(0), s(0), v(0) {}

        inline colorhsv(const color3i& _) : colorhsv(_.r, _.g, _.b) {}
        inline colorhsv(const color4i& _) : colorhsv(_.r, _.g, _.b) {}
        inline colorhsv(const color3f& _) : colorhsv((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}
        inline colorhsv(const color4f& _) : colorhsv((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}

		inline colorhsv(const pixfloat1& _) {
			h = s = 0;
			v = _.x;
		}
		
		inline colorhsv(const pixfloat3& _) : colorhsv(_.r, _.g, _.b) {}

		inline colorhsv(const pixfloat4& _) : colorhsv(_.r, _.g, _.b) {}
		
		inline colorhsv(const pixint1& _) {
			h = s = 0;
			v = (float)_.x / 255;
		}

		inline colorhsv(const pixint3& _) : colorhsv((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}

		inline colorhsv(const pixint4& _) : colorhsv((float)_.r / 255, (float)_.g / 255, (float)_.b / 255) {}

	};

}
#pragma once
#include "../bitmap/pixel_arithmetic.h"
#include <algorithm>
#include <math.h>


namespace Beatmup {

	/**
		HSVA quad
	*/
	struct pixhsva {
		pixfloat h, s, v, a;

		inline pixhsva() : h(0), s(0), v(0), a(1) {}

		inline pixhsva(const pixfloat1& P) {
			h = s = 0;
			v = P.x;
			a = 1.0f;
		}
		
		inline pixhsva(const pixfloat3& P) : pixhsva(P.r, P.g, P.b, 1.0f) {}
		inline pixhsva(const pixfloat4& P) : pixhsva(P.r, P.g, P.b, P.a) {}
		
		inline pixhsva(const pixint1& P) {
			h = s = 0;
			v = (float)P.x / 255;
			a = 1.0f;
		}

		inline pixhsva(const pixint3& P) : pixhsva((float)P.r / 255, (float)P.g / 255, (float)P.b / 255, 1.0f) {}
		inline pixhsva(const pixint4& P) : pixhsva((float)P.r / 255, (float)P.g / 255, (float)P.b / 255, (float)P.a / 255) {}

		/**
			\brief Constructs an HSVA quad from r, g, b, a values
		*/
		inline pixhsva(pixfloat r, pixfloat g, pixfloat b, pixfloat a) : a(a) {
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
			float
				H = h - (long)h + (h >= 0 ? 0 : 1),
				C = v*s,
				X = C*(1 - fabs(modf(H*6, 2) - 1)),
				m = v - C;

			pixfloat4 out;
			if (H < 1.0f / 6) {
				out.r = C + m; out.g = X + m; out.b = m;
			}
			else
				if (H < 2.0f / 6) {
					out.r = X + m; out.g = C + m; out.b = m;
				}
				else
					if (H < 3.0f / 6) {
						out.r = m; out.g = C + m; out.b = X + m;
					}
					else
						if (H < 4.0f / 6) {
							out.r = m; out.g = X + m; out.b = C + m;
						}
						else
							if (H < 5.0f / 6) {
								out.r = X + m; out.g = m; out.b = C + m;
							}
							else {
								out.r = C + m; out.g = m; out.b = X + m;
							}
			out.a = a;
			return out;
		}
	};

}
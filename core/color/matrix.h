/*
	4x4 color matrix
*/
#pragma once
#include "../basic_types.h"
#include "../bitmap/pixel_arithmetic.h"

namespace Beatmup {
	namespace Color {

		class Matrix : public Object {
		public:

			/**
				Matrix elements
			*/
			union {
				pixfloat elem[4][4];
				pixfloat4 rows[4];
			};
#ifdef BEATMUP_CHANNEL_ORDER_ARGB
			inline pixfloat4& a() { return rows[0]; }
			inline pixfloat4& r() { return rows[1]; }
			inline pixfloat4& g() { return rows[2]; }
			inline pixfloat4& b() { return rows[3]; }
#elif BEATMUP_CHANNEL_ORDER_BGRA
			inline pixfloat4& b() { return rows[0]; }
			inline pixfloat4& g() { return rows[1]; }
			inline pixfloat4& r() { return rows[2]; }
			inline pixfloat4& a() { return rows[3]; }
#else // RGBA
			inline pixfloat4& r() { return rows[0]; }
			inline pixfloat4& g() { return rows[1]; }
			inline pixfloat4& b() { return rows[2]; }
			inline pixfloat4& a() { return rows[3]; }
#endif
          
			Matrix();
			pixfloat4& operator[](int);
			pixfloat4 operator[](int) const;
			Matrix operator*(const Matrix&) const;
			void operator=(const Matrix&);

			/**
				Returns color matrix representing an HSV correction transformation
				\param Hdegrees			hue offset in degrees (additive)
				\param S				saturation scale (multiplicative)
				\param V				value scale (multiplicative)
			*/
			static Matrix getHSVCorrection(float Hdegrees, float S, float V);

			/**
				Returns color matrix representing continuous color inversion with a fixed hue point
				\param preservedColor		a color that will be preserved by the transformation;
											if the latter two parameters differ from 1, only hue of this color remains unchanged
				\param Sscale				saturation scale, optional
				\param Vscale				value scale, optional
			*/
			static Matrix getColorInversion(pixfloat3 preservedColor, float Sscale = 1.0f, float Vscale = 1.0f);
		};
	}
}

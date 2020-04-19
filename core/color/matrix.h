/*
    4x4 color matrix
*/
#pragma once
#include "../basic_types.h"

namespace Beatmup {
    namespace Color {

        class Matrix : public Object {
        public:

            /**
                Matrix elements
            */
            union {
                float elem[4][4];
                struct {
                    color4f rows[4];
                };
            };
#ifdef BEATMUP_CHANNEL_ORDER_ARGB
            inline color4f a() const { return rows[0]; }
            inline color4f r() const { return rows[1]; }
            inline color4f g() const { return rows[2]; }
            inline color4f b() const { return rows[3]; }
            inline color4f& a() { return rows[0]; }
            inline color4f& r() { return rows[1]; }
            inline color4f& g() { return rows[2]; }
            inline color4f& b() { return rows[3]; }
#elif BEATMUP_CHANNEL_ORDER_BGRA
            inline color4f b() const { return rows[0]; }
            inline color4f g() const { return rows[1]; }
            inline color4f r() const { return rows[2]; }
            inline color4f a() const { return rows[3]; }
            inline color4f& b() { return rows[0]; }
            inline color4f& g() { return rows[1]; }
            inline color4f& r() { return rows[2]; }
            inline color4f& a() { return rows[3]; }
#else // RGBA
            inline color4f r() const { return rows[0]; }
            inline color4f g() const { return rows[1]; }
            inline color4f b() const { return rows[2]; }
            inline color4f a() const { return rows[3]; }
            inline color4f& r() { return rows[0]; }
            inline color4f& g() { return rows[1]; }
            inline color4f& b() { return rows[2]; }
            inline color4f& a() { return rows[3]; }
#endif
          
            Matrix();
            Matrix(const color4f& r, const color4f& g, const color4f& b, const color4f& a);
            color4f& operator[](int);
            color4f operator[](int) const;
            Matrix operator*(const Matrix&) const;
            void operator=(const Matrix&);

            /**
                Returns color matrix representing an HSV correction transformation
                \param hDegrees			hue offset in degrees (additive)
                \param s				saturation scale (multiplicative)
                \param v				value scale (multiplicative)
            */
            static Matrix getHSVCorrection(float hDegrees, float s, float v);

            /**
                Returns color matrix representing continuous color inversion with a fixed hue point
                \param preservedColor		a color that will be preserved by the transformation.
                                            If the latter two parameters differ from 1, only hue of this color remains unchanged
                \param sScale				saturation scale, optional
                \param vScale				value scale, optional
            */
            static Matrix getColorInversion(const color3f& preservedColor, float sScale = 1.0f, float vScale = 1.0f);
        };
    }
}

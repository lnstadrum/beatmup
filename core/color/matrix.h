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
#include "../basic_types.h"

namespace Beatmup {
    namespace Color {

        /**
            RGBA color mapping.
            A color value is mapped onto another color value by multiplying the 4x4 matrix by an RGBA input column.
        */
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

            /**
                Initializes the color matrix to identity.
            */
            Matrix();

            /**
                Constructs a matrix from four RGBA color values setting them as matrix rows.
                \param r    First row of the color matrix.
                \param g    Second row of the color matrix.
                \param b    Third row of the color matrix.
                \param a    Fourth row of the color matrix.
            */
            Matrix(const color4f& r, const color4f& g, const color4f& b, const color4f& a);

            /**
                Constructs a color matrix representing an HSV correction transformation.
                \param hDegrees             hue offset in degrees (additive)
                \param saturationFactor     saturation scaling factor
                \param valueFactor          value scaling factor
            */
            Matrix(float hDegrees, float saturationFactor = 1.0f, float valueFactor = 1.0f);

            /**
                Constructs a color matrix representing continuous color inversion with a fixed hue point.
                \param preservedColor       color giving a hue value that remains unchanged by the transform.
                \param saturationFactor     saturation scaling factor
                \param valueFactor          value scaling factor
            */
            Matrix(const color3f& preservedColor, float saturationFactor = 1.0f, float valueFactor = 1.0f);

            /**
                Retrieves matrix rows by index in 0..3 range.
            */
            color4f& operator[](int);
            color4f operator[](int) const;

            /**
                Computes the right-multiplication of the current Matrix and another Matrix.
            */
            Matrix operator*(const Matrix&) const;

            void operator=(const Matrix&);
        };
    }
}

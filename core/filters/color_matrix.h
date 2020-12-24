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

#pragma once
#include "pixelwise_filter.h"
#include "../basic_types.h"
#include "../color/matrix.h"

namespace Beatmup {
    namespace Filters {

        /**
            Color matrix filter: applies mapping Ax + B at each pixel of a given image in RGBA space.
        */
        class ColorMatrix : public PixelwiseFilter {
        private:
            Color::Matrix matrix;       //!< the matrix
            color4f bias;				//!< added constant
            bool allowIntApprox;		//!< allow integer approximation of the coefficients

        protected:
            void apply(int x, int y, msize nPix, TaskThread& thread);

            std::string getGlslDeclarations() const;
            std::string getGlslSourceCode() const;
            void setup(bool useGpu);

        public:
            ColorMatrix();

            inline bool isIntegerApproximationsAllowed() const { return allowIntApprox; }

            void allowIntegerApproximations(bool allow);

            inline Color::Matrix& getMatrix() { return matrix; }
        
            /**
                Sets color matrix coefficients for a specific output color channel.
                \param outChannel           matrix line number (output channel)
                \param bias                 constant to add to the output channel
                \param r                    red channel coefficient
                \param g                    green channel coefficient
                \param b                    blue channel coefficient
                \param a                    alpha channel coefficient
            */
            void setCoefficients(int outChannel, float bias, float r = .0f, float g = .0f, float b = .0f, float a = .0f);

            /**
                Resets the current transformation to a matrix performing standard HSV correction.
                \param hueShiftDegrees      Hue shift in degrees
                \param saturationFactor     Saturation scaling factor
                \param valueFactor          Value scaling factor
            */
            void setHSVCorrection(float hueShiftDegrees, float saturationFactor = 1.0f, float valueFactor = 1.0f);

            /**
                Resets the current transformation to a fancy color inversion mode with a fixed hue point.
                \param preservedHue         Hue value to keep constant. The other colors are inverted
                \param saturationFactor     Saturation scaling factor (in HSV sense)
                \param valueFactor          Value scaling factor (in HSV sense)
            */
            void setColorInversion(color3f preservedHue, float saturationFactor = 1.0f, float valueFactor = 1.0f);

            /**
                Applies a contrast adjustment by a given factor on top of the current transformation
            */
            void applyContrast(float factor);

            /**
                Sets a brightness adjustment by a given factor (non-cumulative with respect to the current transformation)
            */
            void setBrightness(float brightness);
        };
    }
}
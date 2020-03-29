/*
    Color matrix filter: applying a matrix transform at each pixel
*/

#pragma once
#include "pixelwise_filter.h"
#include "../../basic_types.h"
#include "../../color/matrix.h"

namespace Beatmup {
    namespace Filters {

        class ColorMatrix : public PixelwiseFilter {
        private:
            Color::Matrix matrix;
            color4f add;				//!< additional vector
            bool allowIntApprox;		//!< allow integer approximation of the coefficients

        public:
            ColorMatrix();
            void apply(int startx, int starty, msize nPix, TaskThread& thread);

            bool isIntegerApproximationsAllowed() const { return allowIntApprox; }

            void allowIntegerApproximations(bool allow);

            Color::Matrix& getMatrix() { return matrix; }
        
            /**
                Sets matrix coefficient
                \param outChannel		matrix line number (output channel)
                \param add				constant to add to the output channel
                \param inR				red channel coefficient
                \param inG				green channel coefficient
                \param inB				blue channel coefficient
                \param inA				alpha channel coefficient
            */
            void setCoefficients(int outChannel, float add, float inR = .0f, float inG = .0f, float inB = .0f, float inA = .0f);

            /**
                Sets the matrix to perform standard HSV correction
                \param addHueDegrees	H offset
                \param scaleSat			S scale
                \param scaleVal			V scale
            */
            void setHSVCorrection(float addHueDegrees, float scaleSat, float scaleVal);

            void setColorInversion(color3f preservedHue, float scaleSat = 1.0f, float scaleVal = 1.0f);
        };
    }
}
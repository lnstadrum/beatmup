#include "color_matrix.h"
#include "../../bitmap/bitmap_access.h"
#include "../../bitmap/processing.h"
#include "../../color/color_spaces.h"
#include "../../color/colors.h"

using namespace Beatmup;

Filters::ColorMatrix::ColorMatrix() : allowIntApprox(true)
{
	add = Colors::ZeroF;
}


template <class in_t, class out_t> class ApplyColorMatrix {
public:
	static void process(in_t in, out_t out, const pixfloat4 &addF, const Color::Matrix& matrixF, bool useIntApprox, msize nPix) {
		if (useIntApprox) {
			pixint4 matrixI[4], addI;
			addI = addF;
			for (int i = 0; i < 4; i++)
				matrixI[i] = matrixF[i];
			for (msize n = 0; n < nPix; n++) {
				out.assign(
					(in() * matrixI[CHANNELS_4.R]).sum() + addI.val[CHANNELS_4.R],
					(in() * matrixI[CHANNELS_4.G]).sum() + addI.val[CHANNELS_4.G],
					(in() * matrixI[CHANNELS_4.B]).sum() + addI.val[CHANNELS_4.B],
					(in() * matrixI[CHANNELS_4.A]).sum() + addI.val[CHANNELS_4.A]
				);
				in++;
				out++;
			}
		}
		else
			for (msize n = 0; n < nPix; n++) {
				out.assign(
					(in() * matrixF[CHANNELS_4.R]).sum() + addF[CHANNELS_4.R],
					(in() * matrixF[CHANNELS_4.G]).sum() + addF[CHANNELS_4.G],
					(in() * matrixF[CHANNELS_4.B]).sum() + addF[CHANNELS_4.B],
					(in() * matrixF[CHANNELS_4.A]).sum() + addF[CHANNELS_4.A]
				);
				in++;
				out++;
			}
	}
};


void Filters::ColorMatrix::apply(int startx, int starty, msize nPix, TaskThread& thread) {
	BitmapProcessing::pipeline<ApplyColorMatrix>(
		*inputBitmap, *outputBitmap, startx, starty,
		add, matrix, allowIntApprox && inputBitmap->isInteger() && outputBitmap->isInteger(),
		nPix
	);
}


void Filters::ColorMatrix::allowIntegerApproximations(bool allow) {
	allowIntApprox = allow;
}


void Filters::ColorMatrix::setCoefficients(int outChannel, float add, float inR, float inG, float inB, float inA) {
	RuntimeError::check(outChannel >= 0 && outChannel <= 3, "Invalid output channel index");
    pixfloat4 _(this->add);
	_[outChannel] = add;
    this->add = _;
	matrix[outChannel].r = inR;
	matrix[outChannel].g = inG;
	matrix[outChannel].b = inB;
	matrix[outChannel].a = inA;
}


void Filters::ColorMatrix::setHSVCorrection(float hDegrees, float s, float v) {
	matrix = Color::Matrix::getHSVCorrection(hDegrees, s, v);
	add = Colors::ZeroF;
}


void Filters::ColorMatrix::setColorInversion(color3f preservedColor, float s, float v) {
	matrix = Color::Matrix::getColorInversion(preservedColor, s, v);
	add = Colors::ZeroF;
}

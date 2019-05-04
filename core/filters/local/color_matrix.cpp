#include "color_matrix.h"
#include "../../bitmap/bitmap_access.h"
#include "../../bitmap/processing.h"
#include "../../color/color_spaces.h"

using namespace Beatmup;

Filters::ColorMatrix::ColorMatrix() : allowIntApprox(true)
{
	add.zero();
}


template <class in_t, class out_t> class ApplyColorMatrix {
public:
	static void process(in_t in, out_t out, pixfloat4 &addF, Color::Matrix& matrixF, bool useIntApprox, msize nPix) {
		if (useIntApprox) {
			pixint4 matrixI[4], addI;
			addI = addF;
			for (int i = 0; i < 4; i++)
				matrixI[i] = matrixF[i];
			for (msize n = 0; n < nPix; n++) {
				out.assign(
					(in() * matrixI[CHANNELS.R]).sum() + addI.val[CHANNELS.R],
					(in() * matrixI[CHANNELS.G]).sum() + addI.val[CHANNELS.G],
					(in() * matrixI[CHANNELS.B]).sum() + addI.val[CHANNELS.B],
					(in() * matrixI[CHANNELS.A]).sum() + addI.val[CHANNELS.A]
				);
				in++;
				out++;
			}
		}
		else
			for (msize n = 0; n < nPix; n++) {
				out.assign(
					(in() * matrixF[CHANNELS.R]).sum() + addF[CHANNELS.R],
					(in() * matrixF[CHANNELS.G]).sum() + addF[CHANNELS.G],
					(in() * matrixF[CHANNELS.B]).sum() + addF[CHANNELS.B],
					(in() * matrixF[CHANNELS.A]).sum() + addF[CHANNELS.A]
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


void Filters::ColorMatrix::setAllowIntegerApproximations(bool allow) {
	allowIntApprox = allow;
}


void Filters::ColorMatrix::setCoefficients(int outChannel, float add, float inR, float inG, float inB, float inA) {
	if (outChannel < 0 || outChannel > 3)
		BEATMUP_ERROR("Wrong output channel index: %d", outChannel);
	this->add.val[outChannel] = add;
	matrix[outChannel][CHANNELS.R] = inR;
	matrix[outChannel][CHANNELS.G] = inG;
	matrix[outChannel][CHANNELS.B] = inB;
	matrix[outChannel][CHANNELS.A] = inA;
}


void Filters::ColorMatrix::setHSVCorrection(float Hdegrees, float S, float V) {
	matrix = Color::Matrix::getHSVCorrection(Hdegrees, S, V);
	add = pixfloat4(0, 0, 0, 0);
}


void Filters::ColorMatrix::setColorInversion(pixfloat3 preservedColor, float S, float V) {
	matrix = Color::Matrix::getColorInversion(preservedColor, S, V);
	add = pixfloat4(0, 0, 0, 0);
}
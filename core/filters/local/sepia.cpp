#include "../../filters/local/sepia.h"
#include "../../bitmap/bitmap_access.h"
#include "../../bitmap/processing.h"
#include <algorithm>

using namespace Beatmup;


template <class in_t, class out_t> class ApplySepia {
public:
	static inline void process(in_t in, out_t out, msize nPix) {
		const pixint3
			R{ 100, 196, 48 },
			G{ 89, 175, 43 },
			B{ 69, 138, 33 };

		for (msize n = 0; n < nPix; n++) {
			pixint4 P = (pixint4)in();
			out.assign(
				(P.r * R.r + P.g * R.g + P.b * R.b) / 255,
				(P.r * G.r + P.g * G.g + P.b * G.b) / 255,
				(P.r * B.r + P.g * B.g + P.b * B.b) / 255,
				P.a);
			in++;
			out++;
		}
	}
};


void Filters::Sepia::apply(int startx, int starty, msize nPix, TaskThread& thread) {
	BitmapProcessing::pipeline<ApplySepia>(*inputBitmap, *outputBitmap, startx, starty, nPix);
}
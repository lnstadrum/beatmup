/*
	Image tuning: set of basic processing operations
*/

#pragma once
#include "../parallelism.h"
#include "../bitmap/abstract_bitmap.h"
#include "../debug.h"

namespace Beatmup {
	namespace Filters {

		class ImageTuning : public AbstractTask {
		private:
			static const int MIN_PIXEL_COUNT_PER_THREAD = 1000;			//!< min number of pixels per thread

			AbstractBitmap *inputBitmap, *outputBitmap;					//!< bitmaps

			float
				hueOffset, saturationFactor, valueFactor,
				contrast, brightness;

			virtual bool process(TaskThread& thread) final;
			virtual void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) final;
			virtual void afterProcessing(ThreadIndex threadCount, bool aborted) final;
		public:
			ImageTuning();
			void setBitmaps(AbstractBitmap *input, AbstractBitmap *output);
			ThreadIndex maxAllowedThreads() const;

			inline void setHueOffset(float val) { hueOffset = val; };
			inline void setSaturationFactor(float val) { saturationFactor = val; };
			inline void setValueFactor(float val) { valueFactor = val; };
			inline void setBrightness(float val) { brightness = val; };
			inline void setContrast(float val) { contrast = val; };

			inline float getHueOffset() const { return hueOffset; };
			inline float getSaturationFactor() const { return saturationFactor; };
			inline float getValueFactor() const { return valueFactor; };
			inline float getBrightness() const { return brightness; };
			inline float getContrast() const { return contrast; };
		};
	}
}
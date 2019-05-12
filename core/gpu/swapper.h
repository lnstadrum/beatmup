/*
	Swapping pixel data from GPU to CPU
*/
#pragma once

#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"

namespace Beatmup {
	class Swapper : public GpuTask {
	private:
		AbstractBitmap* bitmap;

		bool processOnGPU(GraphicPipeline& gpu, TaskThread&);

	public:
		Swapper();
		void setBitmap(AbstractBitmap&);

		static void grabPixels(AbstractBitmap& bitmap);
	};
}
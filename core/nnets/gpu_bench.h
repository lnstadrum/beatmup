/**
    A simple GPU benchmarks
 */

#pragma once

#include "../environment.h"
#include "../gpu/compute_program.h"
#include "../gpu/gpu_task.h"
#include "../bitmap/internal_bitmap.h"

namespace Beatmup {
	namespace NNets {
		class GPUBenchmark : public GpuTask {
		private:
			static const int CONTROL_POINTS_NUM = 100;
			
			Environment& env;
			GL::Object<GL::ComputeProgram>* program;
			InternalBitmap input, output;
			
			int ctrlPointsLocs[CONTROL_POINTS_NUM];
			float ctrlPointsVals[CONTROL_POINTS_NUM];

			float error;	//!< max absolute error over all features
			float score;	//!< score

			void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
			void afterProcessing(ThreadIndex threadCount, bool aborted);
			bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);

		public:
			GPUBenchmark(Environment& env);
			~GPUBenchmark();

			void run();

			float getError() const { return error; }

			/**
				\return the resulting score: number of features passed through a 3x3 depthwise convolutional layer + ReLU per second
			*/
			float getScore() const { return score; }
		};
	}
}

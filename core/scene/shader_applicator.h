/*
	Lightweight task applying a layer shader to a bitmap
*/
#pragma once
#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"
#include "../geometry.h"
#include "layer_shader.h"

namespace Beatmup {

	/**
		A task applying a layer shader to a bitmap
	*/
	class ShaderApplicator : public GPUTask {
	private:
		LayerShader* shader;
		AbstractBitmap *input, *output;
		AffineMapping mapping;

		bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
		void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
		void afterProcessing(ThreadIndex threadCount, bool aborted);

	public:
		ShaderApplicator();
		void setInputBitmap(AbstractBitmap* bitmap);
		void setOutputBitmap(AbstractBitmap* bitmap);
		void setShader(LayerShader* shader);

		AbstractBitmap* getInputBitmap() const { return input; }
		AbstractBitmap* getOutputBitmap() const { return output; }
		LayerShader* getLayerShader() const { return shader; }
	};
}
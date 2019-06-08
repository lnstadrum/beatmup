#include "shader_applicator.h"
#include "../gpu/pipeline.h"
#include "../debug.h"

using namespace Beatmup;

bool ShaderApplicator::processOnGPU(GraphicPipeline &gpu, TaskThread &thread) {
	gpu.setOutput(*output);
	mapping.matrix.setElements(1.0f, 0, 0, input->getAspectRatio());
	shader->prepare(gpu, input, mapping);
	gpu.getRenderingPrograms().blend(false);
	return true;
}


void ShaderApplicator::beforeProcessing(ThreadIndex threadCount, GraphicPipeline *gpu) {
	NullTaskInput::check(input, "input bitmap");
	NullTaskInput::check(output, "output bitmap");
	NullTaskInput::check(shader, "layer shader");
	if (input == output)
		throw Exception("Input and output bitmap cannot be equal");
	input->lockPixels(ProcessingTarget::GPU);
	output->lockPixels(ProcessingTarget::GPU);
}


void ShaderApplicator::afterProcessing(ThreadIndex threadCount, bool aborted) {
	input->unlockPixels();
	output->unlockPixels();
}


ShaderApplicator::ShaderApplicator():
	input(nullptr), output(nullptr), shader(nullptr)
{}


void ShaderApplicator::setInputBitmap(AbstractBitmap *bitmap) {
    input = bitmap;
}


void ShaderApplicator::setOutputBitmap(AbstractBitmap *bitmap) {
    output = bitmap;
}


void ShaderApplicator::setShader(LayerShader *shader) {
    this->shader = shader;
}

#include "shader_applicator.h"
#include "../gpu/pipeline.h"
#include "../debug.h"

using namespace Beatmup;

bool ShaderApplicator::processOnGPU(GraphicPipeline &gpu, TaskThread &thread) {	
	shader->prepare(gpu, input, output, AffineMapping::IDENTITY);
	shader->process(gpu);
}


void ShaderApplicator::beforeProcessing(ThreadIndex threadCount, GraphicPipeline *gpu) {
	NullTaskInput::check(output, "output bitmap");
	NullTaskInput::check(shader, "image shader");
	if (input)
		input->lockPixels(ProcessingTarget::GPU);
	output->lockPixels(ProcessingTarget::GPU);
}


void ShaderApplicator::afterProcessing(ThreadIndex threadCount, bool aborted) {
	if (input)
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


void ShaderApplicator::setShader(ImageShader *shader) {
    this->shader = shader;
}

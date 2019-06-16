/**
	Example of generating a mask using flood fill.
*/

#include <scene/renderer.h>
#include <masking/flood_fill.h>
#include <bitmap/internal_bitmap.h>
#include <bitmap/converter.h>

#include <iostream>

int main(int argc, char* argv[]) {
	Beatmup::Environment env;
	Beatmup::Scene scene;
	Beatmup::SceneRenderer renderer;
	Beatmup::FloodFill floodFill;
	float time;

	Beatmup::InternalBitmap fecamp(env, "images/fecamp.bmp");
	Beatmup::InternalBitmap spiral(env, "images/spiral.bmp");
	Beatmup::InternalBitmap mask(env, Beatmup::PixelFormat::HexMask, spiral.getWidth(), spiral.getHeight());
	Beatmup::InternalBitmap output(env, Beatmup::PixelFormat::TripleByte, fecamp.getWidth(), fecamp.getHeight());
	Beatmup::IntPoint seeds[1] = { Beatmup::IntPoint(300, 20) };

	// floodfill
	floodFill.setInput(spiral);
	floodFill.setOutput(mask);
	floodFill.setSeeds(seeds, 1);
	floodFill.setBorderPostprocessing(Beatmup::FloodFill::BorderMorphology::ERODE, 2, 10);

	mask.zero();
		// It is important to zero the mask before going floodfill. By default it contains random stuff (what has been before
		// in RAM), and floodfill will likely not modify all its pixels.
	std::cout << "Running flood fill... ";
	time = env.performTask(floodFill);
	std::cout << time << " ms" << std::endl;

	// configure renderer
	renderer.setScene(scene);
	renderer.setOutputPixelsFetching(true);
	renderer.setOutputMapping(Beatmup::SceneRenderer::OutputMapping::FIT_WIDTH_TO_TOP);
	renderer.setOutput(output);
	env.warmUpGpu();

	// construct the scene
	{
		Beatmup::Scene::BitmapLayer& l = scene.newBitmapLayer();
		l.setBitmap(&fecamp);
		l.getMapping().setCenterPosition(Beatmup::Point(0.5f, 0.5f));
		l.setModulationColor({ 1, 0, 0, 1 });
	}

	{
		Beatmup::Scene::MaskedBitmapLayer& l = scene.newMaskedBitmapLayer();
		l.setBitmap(&fecamp);
		l.setMask(&mask);
		l.getMapping().setCenterPosition(Beatmup::Point(0.5f, 0.5f));
	}

	std::cout << "Rendering..." << std::endl;
	time = env.performTask(renderer);
	std::cout << "Time: " << time << " ms" << std::endl;

	// saving output
	output.saveBmp("output.bmp");
	return 0;
}

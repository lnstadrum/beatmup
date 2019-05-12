/**
	Basic rendering examples

	Constructs a simple scene of four bitmap layers displaying all the same image using three different shaders
	and a shaped bitmap layer.
*/

#include <bitmap/platform_specific/bitmap.h>
#include <scene/renderer.h>

#include <iostream>

#define STRINGIFY(X) #X

int main(int argc, char* argv[]) {
	Beatmup::Environment env;
	Beatmup::Scene scene;
	Beatmup::SceneRenderer renderer;
	Beatmup::Bitmap fecamp(env, L"images/fecamp.jpg");
	Beatmup::Bitmap output(env, Beatmup::PixelFormat::TripleByte, 4000, 4000);

	// setting up a radial image distortion shader
	Beatmup::LayerShader distortShader(env);
	distortShader.setSourceCode(STRINGIFY(
		#beatmup_input_image image;
		varying vec2 texCoord;
		vec2 distort(vec2 xy) {
			vec2 r = xy - vec2(0.5, 0.5);
			float t = length(r);
			return (-0.5 * t * t + 0.9) * r + vec2(0.5, 0.5);
		}
		void main() {
			gl_FragColor = texture2D(image, distort(texCoord));
		}
	));

	// setting up a color channel shifting  shader
	Beatmup::LayerShader grayShiftShader(env);
	grayShiftShader.setSourceCode(STRINGIFY(
		#beatmup_input_image image;
		varying vec2 texCoord;
		float gray(vec2 pos) {
			vec4 clr = texture2D(image, pos);
			return 0.333 * (clr.r + clr.g + clr.b);
		}
		void main() {
			gl_FragColor = vec4(
				gray(texCoord + vec2(0.01, 0.01)),
				gray(texCoord),
				gray(texCoord - vec2(0.01, 0.01)),
				1.0
			);
		}
	));
	
	// setting up a recoloring shader
	// note: the same may be done by setting modulation color in a regular bitmap layer (the custom shader here is a bit an overkill)
	Beatmup::LayerShader recolorShader(env);
	recolorShader.setSourceCode(STRINGIFY(
		#beatmup_input_image image;
		varying vec2 texCoord;
		void main() {
			gl_FragColor = texture2D(image, texCoord) * vec4(1, 0.8, 0.7, 1);
		}
	));

	// constructing a simple scene
	{
		Beatmup::Scene::ShapedBitmapLayer& l = scene.newShapedBitmapLayer();
		l.getMapping().scale(0.48f);
		l.getMapping().rotateDegrees(1, Beatmup::Point(0.5, 0.5));
		l.getMapping().setCenterPosition(Beatmup::Point(0.25, 0.75));
		l.setBitmap(&fecamp);
		l.setCornerRadius(0.05f);
		l.setSlopeWidth(0.01f);
		l.setInPixels(false);
	}
	
	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.getMapping().scale(0.48f);
		l.getMapping().rotateDegrees(-1, Beatmup::Point(0.5, 0.5));
		l.getMapping().setCenterPosition(Beatmup::Point(0.75, 0.25));
		l.setBitmap(&fecamp);
		l.setLayerShader(&distortShader);
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.getMapping().scale(0.48f);
		l.getMapping().rotateDegrees(-2, Beatmup::Point(0.5, 0.5));
		l.getMapping().setCenterPosition(Beatmup::Point(0.75, 0.75));
		l.setBitmap(&fecamp);
		l.setLayerShader(&grayShiftShader);
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.getMapping().scale(0.45f);
		l.getMapping().rotateDegrees(-3, Beatmup::Point(0.5, 0.5));
		l.getMapping().setCenterPosition(Beatmup::Point(0.25, 0.25));
		l.setBitmap(&fecamp);
		l.setLayerShader(&recolorShader);
	}

	// configuring renderer
	renderer.setScene(scene);
	renderer.setOutputPixelsFetching(true);
	renderer.setOutputMapping(Beatmup::SceneRenderer::OutputMapping::FIT_WIDTH);
	renderer.setOutput(output);

	// warming up
	env.warmUpGpu();

	// go
	std::cout << "Rendering..." << std::endl;
	float time;
	time = env.performTask(renderer);
	std::cout << "  First run: " << time << " ms" << std::endl;
	time = env.performTask(renderer);
	std::cout << "  Second run: " << time << " ms" << std::endl;
		// Second run is faster: it has the shaders compiled and all the bitmap data ready in the GPU memory.
	
	// save output
	output.save(L"output.png");
	return 0;
}
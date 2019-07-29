/**
	Basic rendering examples

	Constructs a simple scene of four bitmap layers displaying all the same image using three different shaders
	and a shaped bitmap layer.
*/

#include <bitmap/internal_bitmap.h>
#include <scene/renderer.h>
#include <bitmap/converter.h>

#include <iostream>
#include <ctime>

#define SHADERCODE(X) \
	"beatmupInputImage image;\n" #X

int main(int argc, char* argv[]) {
	Beatmup::Environment env;
	Beatmup::Scene scene;
	Beatmup::SceneRenderer renderer;
	Beatmup::InternalBitmap fecamp(env, "images/fecamp.bmp");
	Beatmup::InternalBitmap bg(env, "images/bg.bmp");
	Beatmup::InternalBitmap output(env, Beatmup::PixelFormat::QuadByte, 1024, 1024);

	Beatmup::InternalBitmap bitmap1 (env, Beatmup::PixelFormat::SingleByte,  fecamp.getWidth(), fecamp.getHeight());
	Beatmup::InternalBitmap bitmap3 (env, Beatmup::PixelFormat::TripleByte,  fecamp.getWidth(), fecamp.getHeight());
	Beatmup::InternalBitmap bitmap3f(env, Beatmup::PixelFormat::TripleFloat, fecamp.getWidth(), fecamp.getHeight());
	Beatmup::InternalBitmap bitmap4f(env, Beatmup::PixelFormat::QuadFloat,   fecamp.getWidth(), fecamp.getHeight());

	env.limitWorkerCount(1);
	Beatmup::BitmapConverter::convert(fecamp, bitmap3);
	Beatmup::BitmapConverter::convert(fecamp, bitmap1);
	Beatmup::BitmapConverter::convert(fecamp, bitmap3f);
	Beatmup::BitmapConverter::convert(fecamp, bitmap4f);

	// setting up a radial image distortion shader
	Beatmup::ImageShader distortShader(env);
	distortShader.setSourceCode(SHADERCODE(
		varying highp vec2 texCoord;
		vec2 distort(vec2 xy) {
			vec2 r = xy - vec2(0.5, 0.5);
			float t = length(r);
			return (-0.5 * t * t + 0.9) * r + vec2(0.5, 0.5);
		}
		void main() {
			gl_FragColor = texture2D(image, distort(texCoord));
		}
	));

	// setting up a color channel shifting shader
	Beatmup::ImageShader grayShiftShader(env);
	grayShiftShader.setSourceCode(SHADERCODE(
		varying highp vec2 texCoord;
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

	// setting up a recoloring shader (applying a random matrix to RGB triplets)
	Beatmup::ImageShader recolorShader(env);
	recolorShader.setSourceCode(SHADERCODE(
		varying highp vec2 texCoord;
		uniform mediump mat3 matrix;
		void main() {
			gl_FragColor = vec4(matrix * texture2D(image, texCoord).rgb, 1);
		}
	));
	float matrix[9];
	std::srand(std::time(nullptr));
	for (int i = 0; i < 9; ++i) {
		matrix[i] = (float)std::rand() / RAND_MAX;
	}
	recolorShader.setFloatMatrix3("matrix", matrix);

	// constructing a simple scene
	{
		Beatmup::Scene::ShapedBitmapLayer& l = scene.newShapedBitmapLayer();
		l.getMapping().scale(0.48f);
		l.getMapping().rotateDegrees(1);
		l.getMapping().setCenterPosition(Beatmup::Point(0.25, 0.75));
		l.setBitmap(&bitmap1);
		l.setCornerRadius(0.05f);
		l.setSlopeWidth(0.01f);
		l.setInPixels(false);
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.getMapping().scale(0.48f);
		l.getMapping().rotateDegrees(-1);
		l.getMapping().setCenterPosition(Beatmup::Point(0.75, 0.25));
		l.setBitmap(&bitmap4f);
		l.setLayerShader(&distortShader);
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.getMapping().scale(0.48f);
		l.getMapping().rotateDegrees(-2);
		l.getMapping().setCenterPosition(Beatmup::Point(0.75, 0.75));
		l.setBitmap(&bitmap3);
		l.setLayerShader(&grayShiftShader);
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.getMapping().scale(0.45f);
		l.getMapping().rotateDegrees(-3);
		l.getMapping().setCenterPosition(Beatmup::Point(0.25, 0.25));
		l.setBitmap(&bitmap3f);
		l.setLayerShader(&recolorShader);
	}

	// configuring renderer
	renderer.setScene(scene);
	renderer.setBackgroundImage(&bg);
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
	output.saveBmp("output_basic.bmp");
	return 0;
}

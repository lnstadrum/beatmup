#include <stdio.h>

#include <bitmap/platform_specific/bitmap.h>
#include <scene/renderer.h>

#define STRINGIFY(X) #X

int main(int argc, char* argv[]) {
	Beatmup::Environment env;
	Beatmup::Scene scene;
	Beatmup::SceneRenderer renderer;
	renderer.setScene(scene);
	renderer.setOutputPixelsFetching(true);
	renderer.setOutputMapping(Beatmup::SceneRenderer::OutputMapping::FIT_WIDTH);

	// radial image distortion
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

	// channel shifting filter
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
	
	// recoloring filter
	Beatmup::LayerShader recolorShader(env);
	recolorShader.setSourceCode(STRINGIFY(
		#beatmup_input_image image;
		varying vec2 texCoord;
		void main() {
			gl_FragColor = texture2D(image, texCoord) * vec4(1, 0.9, 0.6, 1);
		}
	));

	Beatmup::Bitmap fecamp(env, L"images/fecamp.jpg");
	Beatmup::Bitmap output(env, 4000, 4000, Beatmup::PixelFormat::TripleByte);
	renderer.setOutput(output);

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
	printf("Running tasks...\n");

	float time = env.performTask(renderer);
	printf("Run: %0.2f ms\n", time);

	output.save(L"output.png");
	return 0;
}
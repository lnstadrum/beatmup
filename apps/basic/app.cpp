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
	renderer.setOutputMapping(Beatmup::SceneRenderer::OutputMapping::FIT_WIDTH_TO_TOP);

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

	// horizontal blurring filter
	Beatmup::LayerShader blurringFilter(env);
	blurringFilter.setSourceCode(STRINGIFY(
		#beatmup_input_image image;
		varying vec2 texCoord;
		vec4 blur(vec2 pos) {
			vec3 sum = vec3(0, 0, 0);
			for (int x = -10; x < 10; x++)
				sum += texture2D(image, pos + vec2(0.001 * float(x), 0)).rgb;
			return vec4(sum / 21.0, 1);
		}
		void main() {
			gl_FragColor = blur(texCoord);
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
	Beatmup::Bitmap output(env, 4000, 3000, Beatmup::PixelFormat::TripleByte);
	renderer.setOutput(output);

	// constructing a simple scene
	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.mapping.scale(0.44f);
		l.mapping.rotateDegrees(1, Beatmup::Point(0.5, 0.5));
		l.mapping.setCenterPosition(Beatmup::Point(0.25, 0.75));
		l.bitmap = &fecamp;
		l.layerShader = &blurringFilter;
	}
	
	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.mapping.scale(0.48f);
		l.mapping.rotateDegrees(-1, Beatmup::Point(0.5, 0.5));
		l.mapping.setCenterPosition(Beatmup::Point(0.75, 0.25));
		l.bitmap = &fecamp;
		l.layerShader = &distortShader;
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.mapping.scale(0.48f);
		l.mapping.rotateDegrees(-2, Beatmup::Point(0.5, 0.5));
		l.mapping.setCenterPosition(Beatmup::Point(0.75, 0.75));
		l.bitmap = &fecamp;
		l.layerShader = &grayShiftShader;
	}

	{
		Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
		l.mapping.scale(0.45f);
		l.mapping.rotateDegrees(-3, Beatmup::Point(0.5, 0.5));
		l.mapping.setCenterPosition(Beatmup::Point(0.25, 0.25));
		l.bitmap = &fecamp;
		l.layerShader = &recolorShader;
	}
	printf("Running tasks...\n");

	float time = env.performTask(renderer);
	printf("Run: %0.2f ms\n", time);

	output.save(L"output.png");
	return 0;
}
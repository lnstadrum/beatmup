/**
    A basic rendering example featuring common bitmap operations,
    Scene, SceneRenderer and custom shading.
    Builds up a scene and renders it to a bitmap.
*/

#include <bitmap/internal_bitmap.h>
#include <scene/renderer.h>
#include <bitmap/converter.h>
#include <bitmap/tools.h>
#include <iostream>


int main(int argc, char* argv[]) {

    // Creating a context: it is necessary to have at least one of those.
    Beatmup::Context ctx;

    // Instantiating a scene to render. It will be feed with some content later on.
    Beatmup::Scene scene;

    // A second scene will be added to the first scene as a sub-layer.
    Beatmup::Scene subscene;

    // Instantiating a renderer. This will be feed with the scene to produce an image.
    Beatmup::SceneRenderer renderer;

    // Loading image data to be used to construct a scene
    Beatmup::InternalBitmap fecamp(ctx, "images/fecamp.bmp");
    Beatmup::InternalBitmap bg(ctx, "images/bg.bmp");

    // Initializing a bitmap the scene will be rendered onto.
    // When running GLES 2.0, QuadByte is the optimal format to transfer from GPU memory.
    Beatmup::InternalBitmap output(ctx, Beatmup::PixelFormat::QuadByte, 1024, 1024);

    // Converting input texture to a 1-channel (grayscale) and a 3-channel (RGB) internal bitmaps.
    // No specific reason to do so; it is done here to showcase InternalBitmaps instantiation and BitmapConverter.
    Beatmup::InternalBitmap bitmap1(ctx, Beatmup::PixelFormat::SingleByte,  fecamp.getWidth(), fecamp.getHeight());
    Beatmup::InternalBitmap bitmap3(ctx, Beatmup::PixelFormat::TripleByte,  fecamp.getWidth(), fecamp.getHeight());
    Beatmup::BitmapConverter::convert(fecamp, bitmap3);
    Beatmup::BitmapConverter::convert(fecamp, bitmap1);

    // Creating a chessboard-like pattern
    Beatmup::AbstractBitmap* chess = Beatmup::BitmapTools::chessboard(ctx, 512, 512, 32, Beatmup::PixelFormat::BinaryMask);

    // Producing the inverse of the chessboard to make a fancy pattern later
    Beatmup::InternalBitmap chessInv(ctx, Beatmup::PixelFormat::BinaryMask, 512, 512);
    Beatmup::BitmapTools::invert(*chess, chessInv);

    // The scene will contain custom shaders.
    // Setting up a radial image distortion shader: it stretches the image pulling its corners away of the center.
    Beatmup::ImageShader distortShader(ctx);
    distortShader.setSourceCode(BEATMUP_SHADER_CODE(
        beatmupInputImage image;
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

    // Setting up a color channel shifting shader
    Beatmup::ImageShader grayShiftShader(ctx);
    grayShiftShader.setSourceCode(BEATMUP_SHADER_CODE(
        beatmupInputImage image;
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

    // MAKING UP THE SCENE:
    // creating layers, setting ther position, scale, orientation, filling with content.
    // First layer will simply display a bitmap with fancy rounded corners.
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

    // Second layer will apply the radial distortion shader set up above.
    {
        Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
        l.getMapping().scale(0.48f);
        l.getMapping().rotateDegrees(-1);
        l.getMapping().setCenterPosition(Beatmup::Point(0.75, 0.25));
        l.setBitmap(&bitmap3);
        l.setLayerShader(&distortShader);
    }

    // One more layer goes with the gray shift shader.
    {
        Beatmup::Scene::ShadedBitmapLayer& l = scene.newShadedBitmapLayer();
        l.getMapping().scale(0.48f);
        l.getMapping().rotateDegrees(-2);
        l.getMapping().setCenterPosition(Beatmup::Point(0.75, 0.75));
        l.setBitmap(&bitmap3);
        l.setLayerShader(&grayShiftShader);
    }

    // Last layer contains an entire scene which shows two bitmaps with masks on top.
    // The masks are the chessboard patterns in the opposite phase.
    {
        Beatmup::Scene::SceneLayer& l = scene.addScene(subscene);
        l.getMapping().scale(0.45f);
        l.getMapping().rotateDegrees(-3);
        l.getMapping().setCenterPosition(Beatmup::Point(0.25, 0.25));
        auto& l1 = subscene.newMaskedBitmapLayer();
        l1.setBitmap(&bitmap1);
        l1.setMask(chess);
        auto& l2 = subscene.newMaskedBitmapLayer();
        l2.setBitmap(&bitmap3);
        l2.setMask(&chessInv);
    }

    // The scene is ready now. Passing it to the renderer.
    renderer.setScene(scene);

    // To make it even fancier, set up a background image.
    renderer.setBackgroundImage(&bg);

    // Specify the rendering space coordinates:
    // width is 1, height is scaled to match the aspect ratio of the output bitmap.
    renderer.setOutputMapping(Beatmup::SceneRenderer::OutputMapping::FIT_WIDTH);

    // Specify the output bitmap.
    renderer.setOutput(output);

    // Ready to render.
    float time;
    std::cout << "Rendering..." << std::endl;
    time = ctx.performTask(renderer);
    std::cout << "  First run: " << time << " ms" << std::endl;
    time = ctx.performTask(renderer);
    std::cout << "  Second run: " << time << " ms" << std::endl;
      // Second run is much likely faster: it has the shaders compiled and all the bitmap data ready to be used by GPU.
      // So will be any further render pass if no new bitmaps/shaders are added.

    // Save output to a file.
    output.saveBmp("output_basic.bmp");

    // We do not forget to delete chess here, the only dynamically allocated bitmap.
    delete chess;
    return 0;
}

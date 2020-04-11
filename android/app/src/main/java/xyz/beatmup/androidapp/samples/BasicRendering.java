package xyz.beatmup.androidapp.samples;

import android.app.Activity;

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Imaging.PixelFormat;
import Beatmup.Shading.Shader;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class BasicRendering extends TestSample {
    @Override
    public String getCaption() {
        return "Basic rendering";
    }

    @Override
    public String getDescription() {
        return "A basic rendering example featuring common bitmap operations, Scene, SceneRenderer and custom shading";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException {
        Context context = drawingTask.getContext();
        Scene scene = new Scene();

        Bitmap androidBitmap = Bitmap.decodeStream(context, app.getAssets().open("fecamp.bmp"));

        // render chessboard
        Beatmup.Bitmap chess = context.renderChessboard( 512, 512, 32, PixelFormat.BinaryMask);
        Beatmup.Bitmap chess2 = chess.clone();
        chess2.invert();

        // setting up a radial image distortion shader
        Shader distortShader = new Shader(context);
        distortShader.setSourceCode(
                "beatmupInputImage image;\n" +
                "varying highp vec2 texCoord;\n" +
                "highp vec2 distort(highp vec2 xy) {\n" +
                "  highp vec2 r = xy - vec2(0.5, 0.5);\n" +
                "  highp float t = length(r);\n" +
                "  return (-0.5 * t * t + 0.9) * r + vec2(0.5, 0.5);\n" +
                "}\n" +
                "void main() {\n" +
                "  gl_FragColor = texture2D(image, distort(texCoord));\n" +
                "}"
        );

        // setting up a color channel shifting shader
        Shader grayShiftShader = new Shader(context);
        grayShiftShader.setSourceCode(
                "beatmupInputImage image;\n" +
                "varying highp vec2 texCoord;\n" +
                "highp float gray(highp vec2 pos) {\n" +
                "  highp vec4 clr = texture2D(image, pos);\n" +
                "  return 0.333 * (clr.r + clr.g + clr.b);\n" +
                "}\n" +
                "void main() {\n" +
                "  gl_FragColor = vec4(\n" +
                "    gray(texCoord + vec2(0.01, 0.01)),\n" +
                "    gray(texCoord),\n" +
                "    gray(texCoord - vec2(0.01, 0.01)),\n" +
                "    1.0);\n" +
                "}");

        Beatmup.Bitmap bitmap1 = context.copyBitmap(androidBitmap, PixelFormat.SingleByte);
        Beatmup.Bitmap bitmap3 = context.copyBitmap(androidBitmap, PixelFormat.TripleByte);
        Beatmup.Bitmap bitmap3f = context.copyBitmap(androidBitmap, PixelFormat.TripleFloat);
        Beatmup.Bitmap bitmap4f = context.copyBitmap(androidBitmap, PixelFormat.QuadFloat);

        {
            Scene.ShapedBitmapLayer layer = scene.newShapedBitmapLayer();
            layer.setBitmap(bitmap1);
            layer.scale(0.48f);
            layer.rotate(1);
            layer.setCenterPosition(0.25f, 0.75f);
            layer.setCornerRadius(0.05f);
            layer.setSlopeWidth(0.01f);
            layer.setInPixels(false);
        }

        {
            Scene.ShadedBitmapLayer layer = scene.newShadedBitmapLayer();
            layer.setShader(distortShader);
            layer.setBitmap(bitmap4f);
            layer.scale(0.48f);
            layer.rotate(-1);
            layer.setCenterPosition(0.75f, 0.25f);
        }

        {
            Scene.ShadedBitmapLayer layer = scene.newShadedBitmapLayer();
            layer.setShader(grayShiftShader);
            layer.setBitmap(bitmap3);
            layer.scale(0.48f);
            layer.rotate(-2);
            layer.setCenterPosition(0.75f, 0.75f);
        }

        {
            Scene subscene = new Scene();
            Scene.SceneLayer layer = scene.newSceneLayer(subscene);
            layer.scale(0.45f);
            layer.rotate(-3);
            layer.setCenterPosition(0.25f, 0.25f);

            Scene.MaskedBitmapLayer l1 = subscene.newMaskedBitmapLayer();
            l1.setMask(chess);
            l1.setBitmap(bitmap1);

            Scene.MaskedBitmapLayer l2 = subscene.newMaskedBitmapLayer();
            l2.setMask(chess2);
            l2.setBitmap(bitmap3f);
        }

        return scene;
    }
}

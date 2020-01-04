package xyz.beatmup.android.app.samples;

import android.app.Activity;

import java.io.IOException;

import Beatmup.Android.Camera;
import Beatmup.Imaging.Color;
import Beatmup.Imaging.ColorMatrix;
import Beatmup.Rendering.Scene;
import Beatmup.Shading.Shader;
import Beatmup.Task;

public class BasicCameraUse extends TestSample {
    @Override
    public String getCaption() {
        return "Camera";
    }

    public String getDescription() {
        return "Camera usage example. A layer displaying the camera image and a shader layer applying a fancy transformation to it.";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera) throws IOException {
        Scene scene = new Scene();

        Scene.BitmapLayer layer1 = scene.newBitmapLayer();
        layer1.setBitmap(camera.getImage());
        layer1.rotate(90);
        layer1.scale(0.5f);
        layer1.setCenterPosition(0.25f, 0.25f);

        Scene.ShadedBitmapLayer layer2 = scene.newShadedBitmapLayer();
        layer2.setBitmap(camera.getImage());
        layer2.rotate(90);
        layer2.scale(0.5f);
        layer2.setCenterPosition(0.75f, 0.75f);

        layer2.setShader(new Shader(drawingTask.getContext()));
        ColorMatrix matrix = new ColorMatrix();
        matrix.setColorInversion(Color.GREEN, 1, 1);
        layer2.getShader().setColorMatrix("transform", matrix);

        layer2.getShader().setSourceCode(
                "beatmupInputImage image;" +
                "varying mediump vec2 texCoord;" +
                "uniform mediump mat4 transform;" +
                "void main() {" +
                "   highp vec2 xy = vec2(texCoord.x * texCoord.x, texCoord.y);" +
                "   gl_FragColor.rgba = transform * texture2D(image, xy).rgba;" +
                "}"
        );
        return scene;
    }

    @Override
    public boolean usesCamera() {
        return true;
    }
}

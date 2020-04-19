package xyz.beatmup.androidapp.samples;

import android.app.Activity;

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Imaging.Color;
import Beatmup.Imaging.ColorMatrix;
import Beatmup.Pipelining.Multitask;
import Beatmup.Pipelining.TaskHolder;
import Beatmup.Rendering.Scene;
import Beatmup.Shading.Shader;
import Beatmup.Shading.ShaderApplicator;
import Beatmup.Task;

public class MultitaskRendering extends TestSample {
    private Multitask multitask;
    private TaskHolder shaderApplication;

    @Override
    public String getCaption() {
        return "Multitask rendering";
    }

    @Override
    public String getDescription() {
        return "Putting a color matrix shader and a renderer in a rendering task. The color matrix is only applied when updated (tap the image).";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException {
        Context context = drawingTask.getContext();
        Bitmap bitmap = Bitmap.decodeStream(context, app.getAssets().open("fecamp.bmp"));

        // set up a shader applying a color matrix to the image
        ShaderApplicator applicator = new ShaderApplicator(context);
        applicator.addSampler(bitmap);
        applicator.setOutput(Beatmup.Bitmap.createEmpty(bitmap));
        applicator.setShader(new Shader(context));
        applicator.getShader().setSourceCode(
                "beatmupInputImage image;" +
                "varying mediump vec2 texCoord;" +
                "uniform mediump mat4 transform;" +
                "uniform highp float dx;" +
                "uniform highp float dy;" +
                "void main() {" +
                "   highp vec3 clr = texture2D(image, texCoord.xy).rgb;" +
                "   gl_FragColor.rgba = transform * vec4(clr, 1.0);" +
                "}"
        );
        applicator.getShader().setFloat("dx", 1.0f/bitmap.getWidth());
        applicator.getShader().setFloat("dy", 1.0f/bitmap.getHeight());

        // set color inversion preserving the sky color
        ColorMatrix matrix = new ColorMatrix();
        matrix.setColorInversion(Color.FECAMP_SKY, 1, 1);
        applicator.getShader().setColorMatrix("transform", matrix);

        // set up a scene
        Scene scene = new Scene();
        Scene.BitmapLayer bitmapLayer = scene.newBitmapLayer();
        bitmapLayer.setBitmap(applicator.getOutput());
        bitmapLayer.scale(0.9f);
        bitmapLayer.setCenterPosition(0.5f, 0.5f);
        bitmapLayer.setName("Switch color");

        // feed mutitask with the shader application and the drawing task
        multitask = new Multitask(context);
        shaderApplication = multitask.addTask(applicator, Multitask.RepetitionPolicy.REPEAT_UPDATE);
        multitask.addTask(drawingTask);
        multitask.measure();

        return scene;
    }

    @Override
    public Task getDrawingTask() {
        return multitask;
    }

    @Override
    public void onTap(Scene.Layer layer) {
        if (layer.getName().equals("Switch color")) {
            ColorMatrix matrix = new ColorMatrix();
            matrix.setColorInversion(
                    Color.byHue((float) Math.random() * 360.0f), 1, 1
            );
            ((ShaderApplicator)shaderApplication.getTask()).getShader().setColorMatrix("transform", matrix);
            multitask.setRepetitionPolicy(shaderApplication, Multitask.RepetitionPolicy.REPEAT_UPDATE);
        }
    }

}

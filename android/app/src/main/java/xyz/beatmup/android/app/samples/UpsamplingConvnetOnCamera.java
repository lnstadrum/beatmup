package xyz.beatmup.android.app.samples;

import android.app.Activity;

import java.io.IOException;

import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Imaging.Filters.Resampler;
import Beatmup.Imaging.PixelFormat;
import Beatmup.Pipelining.Multitask;
import Beatmup.Pipelining.TaskHolder;
import Beatmup.Rendering.Scene;
import Beatmup.Shading.Shader;
import Beatmup.Shading.ShaderApplicator;
import Beatmup.Task;

public class UpsamplingConvnetOnCamera extends TestSample {
    private Resampler resampler;
    private ShaderApplicator textureCopy;
    private Multitask multitask;
    private TaskHolder resamplerTaskHolder;
    private String info = "";

    @Override
    public String getCaption() {
        return "Upsampling camera with CNN";
    }

    @Override
    public String getDescription() {
        return "Inferring 2x upsampling neural net on the camera preview.";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera) throws IOException {
        Context context = drawingTask.getContext();

        int width =  camera.getResolution().getWidth();
        int height = camera.getResolution().getHeight();
        Beatmup.Bitmap input  = new Beatmup.Bitmap(context, width, height, PixelFormat.TripleByte);
        Beatmup.Bitmap output = new Beatmup.Bitmap(context, 2 * width, 2 * height, PixelFormat.TripleByte);
        info = String.format("%dx%d -> %dx%d", width, height, 2 * width, 2 * height);

        // Create a dummy shader copying camera texture onto another texture.
        // This is done because the upsampling may take a significant time, and when reconstructing
        // the output from upsampled luma and input chroma, the camera frame may be change in the
        // meantime.
        textureCopy = new ShaderApplicator(context);
        textureCopy.addSampler(camera.getImage());
        textureCopy.setOutput(input);
        textureCopy.setShader(new Shader(context));
        textureCopy.getShader().setSourceCode(
                "beatmupInputImage image;" +
                "varying highp vec2 texCoord;" +
                "void main() {" +
                        " gl_FragColor = texture2D(image, texCoord);" +
                "}"
        );

        // init convnet resampler
        resampler = new Resampler(context);
        resampler.setMode(Resampler.Mode.CONVNET);
        resampler.setBitmaps(input, output);

        // put all this together in a Multitask
        multitask = new Multitask(context);
        multitask.addTask(textureCopy, Multitask.RepetitionPolicy.REPEAT_ALWAYS);
        resamplerTaskHolder = multitask.addTask(resampler, Multitask.RepetitionPolicy.REPEAT_ALWAYS);
        multitask.addTask(drawingTask, Multitask.RepetitionPolicy.REPEAT_ALWAYS);
        multitask.measure();

        // setup a scene scene
        int orientation = camera.getOrientation(app.getWindowManager().getDefaultDisplay());
        AffineMapping imgTransform = new AffineMapping();
        imgTransform.rotateAround(0.5f, 0.5f * height / width, orientation);

        int outWidth = app.getWindow().getDecorView().getWidth();
        int outHeight = app.getWindow().getDecorView().getHeight();
        float ar = (float)outWidth / outHeight;

        Scene scene = new Scene();
        {
            Scene.BitmapLayer l = scene.newBitmapLayer();
            l.setCenterPosition(0.5f, 0.25f / ar);
            l.setImageTransform(imgTransform);
            l.setBitmap(input);
        }

        {
            Scene.BitmapLayer l = scene.newBitmapLayer();
            l.setCenterPosition(0.5f, 0.75f / ar);
            l.setImageTransform(imgTransform);
            l.setBitmap(output);
        }

        return scene;
    }

    @Override
    public Task getDrawingTask() {
        return multitask;
    }

    @Override
    public boolean usesCamera() {
        return true;
    }

    @Override
    public String getRuntimeInfo() {
        if (resamplerTaskHolder != null)
            return String.format("%s, %.2f FPS", info, 1000 / resamplerTaskHolder.getRunTime());
        else
            return "";
    }
}

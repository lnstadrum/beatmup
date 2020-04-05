package xyz.beatmup.android.app.samples;

import android.app.Activity;
import android.util.Log;

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Imaging.Filters.Resampler;
import Beatmup.Imaging.PixelFormat;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class UpsamplingConvnet extends TestSample {
    String runtimeInfo;

    @Override
    public String getCaption() {
        return "Upsampling with CNN";
    }

    @Override
    public String getDescription() {
        return "Comparison of CNN-based 2x upsampling and bicubic interpolation on an image.";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera) throws IOException {
        Context context = drawingTask.getContext();
        Scene scene = new Scene();

        Bitmap input = Bitmap.decodeStream(context, app.getAssets().open("butterfly.bmp"));
        Beatmup.Bitmap outputCubic   = new Beatmup.Bitmap(context, 2 * input.getWidth(),  2 * input.getHeight(), PixelFormat.QuadByte);
        Beatmup.Bitmap outputConvnet = new Beatmup.Bitmap(context, 2 * input.getWidth(),  2 * input.getHeight(), PixelFormat.QuadByte);

        Resampler resampler = new Resampler(context);
        resampler.setMode(Resampler.Mode.CUBIC);
        resampler.setBitmaps(input, outputCubic);
        float time = resampler.execute();
        Log.i("Beatmup", String.format("Bicubic resampling: %f ms", time));

        resampler.setMode(Resampler.Mode.CONVNET);
        Beatmup.Bitmap dummy = Beatmup.Bitmap.createEmpty(input);
        resampler.setBitmaps(dummy, outputConvnet);
        float prepTime = resampler.execute();
        Log.i("Beatmup", String.format("Preparing shaders: %f ms", time));

        resampler.setBitmaps(input, outputConvnet);
        Bitmap.recycle(dummy);
        float infTime = resampler.execute();
        Log.i("Beatmup", String.format("Convnet resampling: %f ms", time));

        runtimeInfo = String.format("Preparation : %.2f, inference: %.2f ms", prepTime, infTime);

        {
            Scene.BitmapLayer l = scene.newBitmapLayer();
            l.scale(0.5f);
            l.setCenterPosition(0.5f, 0.25f);
            l.setBitmap(outputCubic);
        }

        {
            Scene.BitmapLayer l = scene.newBitmapLayer();
            l.scale(0.5f);
            l.setCenterPosition(0.5f, 0.75f);
            l.setBitmap(outputConvnet);
        }

        return scene;
    }


    @Override
    public String getRuntimeInfo() {
        return runtimeInfo;
    }
}

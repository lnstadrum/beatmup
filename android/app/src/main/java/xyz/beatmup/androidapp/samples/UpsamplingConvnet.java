/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package xyz.beatmup.androidapp.samples;

import android.app.Activity;
import android.util.Log;

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Exceptions.CoreException;
import Beatmup.Imaging.Resampler;
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
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException, CoreException {
        Context context = drawingTask.getContext();
        Scene scene = new Scene();

        Bitmap input = Bitmap.decodeStream(context, app.getAssets().open("butterfly.bmp"));
        Beatmup.Bitmap outputCubic   = new Beatmup.Bitmap(context, 2 * input.getWidth(),  2 * input.getHeight(), PixelFormat.QuadByte);
        Beatmup.Bitmap outputConvnet = new Beatmup.Bitmap(context, 2 * input.getWidth(),  2 * input.getHeight(), PixelFormat.QuadByte);

        Resampler resampler = new Resampler(context);
        resampler.setMode(Resampler.Mode.CUBIC);
        resampler.setInput(input);
        resampler.setOutput(outputCubic);
        float time = resampler.execute();
        Log.i("Beatmup", String.format("Bicubic resampling: %f ms", time));

        resampler.setMode(Resampler.Mode.CONVNET);
        Beatmup.Bitmap dummy = Beatmup.Bitmap.createEmpty(input);
        resampler.setInput(dummy);
        resampler.setOutput(outputConvnet);
        float prepTime = resampler.execute();
        Log.i("Beatmup", String.format("Preparing shaders: %f ms", time));

        resampler.setInput(input);
        resampler.setOutput(outputConvnet);
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

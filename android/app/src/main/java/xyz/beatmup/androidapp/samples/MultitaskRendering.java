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

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Imaging.Filters.ColorMatrixTransform;
import Beatmup.Pipelining.Multitask;
import Beatmup.Pipelining.TaskHolder;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class MultitaskRendering extends TestSample {
    private Multitask multitask;
    private TaskHolder cmtHolder;

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

        // set up a color matrix transform
        ColorMatrixTransform cmt = new ColorMatrixTransform(context);
        cmt.setInput(bitmap);
        cmt.setOutput(Beatmup.Bitmap.createEmpty(bitmap));
        cmt.setHSVCorrection(0, 2, 1);

        // set up a scene
        Scene scene = new Scene();
        Scene.BitmapLayer bitmapLayer = scene.newBitmapLayer();
        bitmapLayer.setBitmap(cmt.getOutput());
        bitmapLayer.scale(0.9f);
        bitmapLayer.setCenterPosition(0.5f, 0.5f);
        bitmapLayer.setName("Switch color");

        // feed multitask with the color matrix transform and the drawing task
        multitask = new Multitask(context);
        cmtHolder = multitask.addTask(cmt, Multitask.RepetitionPolicy.REPEAT_UPDATE);
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
            ((ColorMatrixTransform)cmtHolder.getTask()).setHSVCorrection((float) Math.random() * 360.0f, (float) Math.random() + 0.5f, (float) Math.random() + 0.5f);
            multitask.setRepetitionPolicy(cmtHolder, Multitask.RepetitionPolicy.REPEAT_UPDATE);
        }
    }

}

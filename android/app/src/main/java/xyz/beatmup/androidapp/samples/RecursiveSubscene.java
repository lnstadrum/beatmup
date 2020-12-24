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
import Beatmup.Geometry.AffineMapping;
import Beatmup.Imaging.Color;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class RecursiveSubscene extends TestSample {
    @Override
    public String getCaption() {
        return "Recursive subscene rendering";
    }

    @Override
    public String getDescription() {
        return "A weird rendering example containing scene that contains a bitmap and itself";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException {
        Scene scene = new Scene();
        Bitmap bitmap = Bitmap.decodeStream(drawingTask.getContext(), app.getAssets().open("fecamp.bmp"));

        Scene.BitmapLayer layer = scene.newBitmapLayer();
        layer.setBitmap(bitmap);
        layer.setModulationColor(new Color(255, 255, 255, 192));

        Scene.SceneLayer subscene = scene.newSceneLayer(scene);
        AffineMapping mapping = new AffineMapping();
        mapping.rotateAround(0.5f, 0.5f, 360.0f/255);
        mapping.scale(0.99f);
        subscene.setTransform(mapping);
        subscene.setCenterPosition(0.5f, 0.5f);
        return scene;
    }
}

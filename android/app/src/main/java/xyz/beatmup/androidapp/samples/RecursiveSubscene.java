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

package xyz.beatmup.android.app.samples;

import android.app.Activity;

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Audio.Playback;
import Beatmup.Audio.SampleFormat;
import Beatmup.Audio.Source.Harmonic;
import Beatmup.Context;
import Beatmup.Exceptions.PlaybackException;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Imaging.PixelFormat;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class HarmonicPlayback extends TestSample {
    Playback playback;
    Harmonic harmonic;

    @Override
    public String getCaption() { return "Audio playback: harmonic"; }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera) throws IOException {
        Context context = drawingTask.getContext();
        harmonic = new Harmonic();
        playback = new Playback(context);
        playback.setSource(harmonic);
        try {
            playback.configure(SampleFormat.Int16, 44100, 1, 2, 256);
            playback.start();
            context.submitPersistentTask(playback, 1);
        } catch (PlaybackException e) {
            e.printStackTrace();
        }

        Scene scene = new Scene();
        Bitmap androidBitmap = Bitmap.decodeStream(context, app.getAssets().open("kitten.jpg"));
        Scene.BitmapLayer layer = scene.newBitmapLayer();
        layer.setBitmap(androidBitmap);
        layer.setPosition(0.1f, 0.1f);
        layer.scale(0.25f);
        layer.rotate(10);
        return scene;
    }

    @Override
    public void onGesture(Scene.Layer layer, AffineMapping mapping) {
        harmonic.setFrequency(layer.getX() * 2000);
        harmonic.setAmplitude(layer.getY() * 0.5f);
    }
}

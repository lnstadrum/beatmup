package xyz.beatmup.android.app.samples;

import android.app.Activity;
import android.os.Environment;
import android.util.Log;

import java.io.IOException;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Audio.Playback;
import Beatmup.Audio.SampleFormat;
import Beatmup.Audio.Signal;
import Beatmup.Context;
import Beatmup.Exceptions.PlaybackException;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class WavFilePlayback extends TestSample {
    Playback playback;

    @Override
    public String getCaption() {
        return "WAV file playback";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera) throws IOException {
        Log.i("External storage path", Environment.getExternalStorageDirectory().getAbsolutePath());

        Context context = drawingTask.getContext();

        Signal wav = new Signal(context, "/storage/emulated/0/test.wav");
        Signal.Source source = new Signal.Source(context, wav);
        playback = new Playback(context);
        playback.setSource(source);
        try {
            playback.configure(SampleFormat.Int16, 44100, 2, 2, 1024);
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
}

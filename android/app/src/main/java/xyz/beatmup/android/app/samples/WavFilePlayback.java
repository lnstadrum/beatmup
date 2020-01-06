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
import Beatmup.Audio.SignalPlot;
import Beatmup.Context;
import Beatmup.Exceptions.PlaybackException;
import Beatmup.Geometry.IntRectangle;
import Beatmup.Imaging.Color;
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

        SignalPlot plot = new SignalPlot(context);
        plot.setSignal(wav);
        plot.setBitmap(Bitmap.createColorBitmap(context, 1024, 400));
        plot.setPlotArea(plot.getBitmap().clientRectangle());
        plot.setWindow(new IntRectangle(0, -32000, 123456, 32000), 1);
        plot.setPalette(Color.WHITE, Color.byHue(123), Color.byHue(133));

        long start = System.currentTimeMillis();
        plot.prepareMetering();
        long preparingTime = System.currentTimeMillis() - start;

        float renderingTime = plot.execute();
        Log.i("Beatmup", String.format("Metering preparation: %d msec, plotting: %f msec", (int)preparingTime, renderingTime));

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
        Scene.BitmapLayer layer = scene.newBitmapLayer();
        layer.setBitmap(plot.getBitmap());
        return scene;
    }
}

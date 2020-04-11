package xyz.beatmup.androidapp.samples;

import android.app.Activity;
import android.app.job.JobInfo;
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
    private int playbackJob;
    private Context context;

    long plotPrepareTime;
    float plotRenderTime;

    @Override
    public String getCaption() {
        return "WAV file playback";
    }

    @Override
    public String getDescription() {
        return "Plays a WAV file and plots its magnitude graph.";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException {
        context = drawingTask.getContext();

        Signal wav = new Signal(context, extFile);

        SignalPlot plot = new SignalPlot(context);
        plot.setSignal(wav);
        plot.setBitmap(Bitmap.createColorBitmap(context, 2048, 400));
        plot.setPlotArea(plot.getBitmap().clientRectangle());
        plot.setWindow(new IntRectangle(0, -0x8000, (int)wav.getLength(), 0x7fff), 1);
        plot.setPalette(Color.WHITE, Color.byHue(123), Color.byHue(133));

        long start = System.currentTimeMillis();
        plot.prepareMetering();
        plotPrepareTime = System.currentTimeMillis() - start;
        plotRenderTime = plot.execute();

        Signal.Source source = new Signal.Source(context, wav);
        Playback playback = new Playback(context);
        playback.setSource(source);
        try {
            playback.configure(SampleFormat.Int16, 44100, 2, 2, 1024);
            playback.start();
            playbackJob = context.submitPersistentTask(playback, 1);
        } catch (PlaybackException e) {
            e.printStackTrace();
        }

        Scene scene = new Scene();
        Scene.BitmapLayer layer = scene.newBitmapLayer();
        layer.setBitmap(plot.getBitmap());
        return scene;
    }

    @Override
    public String usesExternalFile() {
        return "audio/wav";
    }

    @Override
    public void stop() {
        context.abortJob(playbackJob, 1);
    }

    @Override
    public String getRuntimeInfo() {
        return String.format("Metering preparation: %d ms, plotting: %.1f ms", (int)plotPrepareTime, plotRenderTime);
    }
}

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
import Beatmup.Exceptions.CoreException;
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
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException, CoreException {
        context = drawingTask.getContext();

        Signal wav = new Signal(context, extFile);

        SignalPlot plot = new SignalPlot(context);
        plot.setSignal(wav);
        plot.setBitmap(Bitmap.createColorBitmap(context, 2048, 400));
        plot.setPlotArea(plot.getBitmap().clientRectangle());
        plot.setWindow(new IntRectangle(0, -0x8000, (int)wav.getDuration(), 0x7fff), 1);
        plot.setPalette(Color.WHITE, Color.byHue(123), Color.byHue(133));

        long start = System.currentTimeMillis();
        plot.prepareMetering();
        plotPrepareTime = System.currentTimeMillis() - start;
        plotRenderTime = plot.execute();

        Signal.Source source = new Signal.Source(context, wav);
        Playback playback = new Playback(context);
        playback.setSource(source);
        try {
            playback.initialize(44100, SampleFormat.Int16, 2, 1024, 2);
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

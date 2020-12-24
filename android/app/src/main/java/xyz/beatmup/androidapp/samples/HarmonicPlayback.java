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
import Beatmup.Audio.Playback;
import Beatmup.Audio.SampleFormat;
import Beatmup.Audio.HarmonicSource;
import Beatmup.Context;
import Beatmup.Exceptions.PlaybackException;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class HarmonicPlayback extends TestSample {
    private HarmonicSource harmonic;
    private int playbackJob;
    private Context context;

    @Override
    public String getCaption() { return "Audio playback: harmonic"; }

    @Override
    public String getDescription() {
        return "Produces a sinusoidal cheep whose frequency and amplitude is controlled by image position";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException {
        context = drawingTask.getContext();
        harmonic = new HarmonicSource();
        Playback playback = new Playback(context);
        playback.setSource(harmonic);
        try {
            playback.initialize(44100, SampleFormat.Int16, 1, 256, 2);
            playback.start();
            playbackJob = context.submitPersistentTask(playback, 1);
        } catch (PlaybackException e) {
            e.printStackTrace();
        }

        Scene scene = new Scene();
        Bitmap androidBitmap = Bitmap.decodeStream(context, app.getAssets().open("fecamp.bmp"));
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
        harmonic.setAmplitude(Math.min(0.75f, layer.getY() * 0.5f));
    }

    @Override
    public void stop() {
        context.abortJob(playbackJob, 1);
    }
}

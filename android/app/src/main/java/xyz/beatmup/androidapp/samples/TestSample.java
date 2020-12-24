/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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

import Beatmup.Android.Camera;
import Beatmup.Exceptions.CoreException;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

/**
 * Created by HomePlaneR on 15/02/2016.
 */
public abstract class TestSample {
    public TestSample() {}


    /**
     * @return test sample title.
     */
    public abstract String getCaption();


    /**
     * @return a detailed description of the test sample.
     */
    public String getDescription() {
        return "";
    }

    /**
     * Main stuff. Constructs a {@link Scene} to be given to {@link Beatmup.Rendering.SceneRenderer}
     * to illustrate a particular feature.
     * @param drawingTask   Task to be executed to render the scene
     * @param app           Application activity
     * @param camera        Camera object used by the scene
     *
     * @return the scene.
     * @throws IOException if cannot load some images
     */
    public abstract Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException, CoreException;

    /**
     * Called when the test sample is being replaced by another test sample
     */
    public void stop() {}

    @Override
    public String toString() {
        return getCaption();
    }

    /**
     * @return `true` if the test sample uses camera.
     */
    public boolean usesCamera() { return false; }

    /**
     * @return MIME type of an external file to be used in the test; null if not needed.
     */
    public String usesExternalFile() { return null; }

    /**
     * @return test sample drawing task. If null is given, the default renderer rendering the scene
     * will be used by the application as the drawing task.
     */
    public Task getDrawingTask() { return null; }

    /**
     * @return a piece of text to be displayed in runtime. Will be called from time to time.
     */
    public String getRuntimeInfo() { return ""; }

    /**
     * Called by the application when the user taps a layer in the scene.
     * @param layer     The layer tapped
     */
    public void onTap(Scene.Layer layer) {}

    /**
     * Called by the application on a user gesture performed with a layer in the scene.
     * @param layer     The layer tapped
     */
    public void onGesture(Scene.Layer layer, AffineMapping mapping) {}
}

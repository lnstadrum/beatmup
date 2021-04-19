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

import Beatmup.Android.Camera;
import Beatmup.Imaging.Color;
import Beatmup.Imaging.ColorMatrix;
import Beatmup.Rendering.Scene;
import Beatmup.Shading.ImageShader;
import Beatmup.Task;

public class BasicCameraUse extends TestSample {
    @Override
    public String getCaption() {
        return "Camera";
    }

    public String getDescription() {
        return "A layer displaying the camera image and a shader layer applying a fancy transformation to it.";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) {
        Scene scene = new Scene();

        Scene.BitmapLayer layer1 = scene.newBitmapLayer();
        layer1.setBitmap(camera.getImage());
        layer1.rotate(90);
        layer1.scale(0.5f);
        layer1.setCenterPosition(0.25f, 0.25f);

        Scene.ShadedBitmapLayer layer2 = scene.newShadedBitmapLayer();
        layer2.setBitmap(camera.getImage());
        layer2.rotate(90);
        layer2.scale(0.5f);
        layer2.setCenterPosition(0.75f, 0.75f);

        layer2.setShader(new ImageShader(drawingTask.getContext()));
        layer2.getShader().setColorMatrix("transform", new ColorMatrix(Color.GREEN, 1.0f, 1.0f));

        layer2.getShader().setSourceCode(
                "uniform beatmupSampler image;" +
                "varying mediump vec2 texCoord;" +
                "uniform mediump mat4 transform;" +
                "void main() {" +
                "   highp vec2 xy = vec2(texCoord.x * texCoord.x, texCoord.y);" +
                "   gl_FragColor.rgba = transform * beatmupTexture(image, xy).rgba;" +
                "}"
        );
        return scene;
    }

    @Override
    public boolean usesCamera() {
        return true;
    }
}

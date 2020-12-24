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
import java.util.ArrayList;

import Beatmup.Android.Bitmap;
import Beatmup.Android.Camera;
import Beatmup.Exceptions.CoreException;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Geometry.IntPoint;
import Beatmup.Geometry.Rectangle;
import Beatmup.Imaging.FloodFill;
import Beatmup.Imaging.PixelFormat;
import Beatmup.Rendering.Scene;
import Beatmup.Task;

public class OptimizedMaskFromBitmap extends TestSample {
    @Override
    public String getCaption() {
        return "Optimized mask from bitmap";
    }

    @Override
    public String getDescription() {
        return "Illustrates the use of 1-, 2- and 4-bit masks.";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException, CoreException {
        Scene scene = new Scene();

        Beatmup.Bitmap heart = Bitmap.decodeStream(drawingTask.getContext(), app.getAssets().open("heart.png"));

        // preparing masker
        FloodFill masker = new FloodFill(drawingTask.getContext());
        masker.setInput(heart);
        masker.setBorderPostprocessing(FloodFill.BorderMorphology.DILATE, 1, 40);
        ArrayList<IntPoint> seeds = new ArrayList<>();
        seeds.add(new IntPoint(heart.getWidth() / 2, heart.getHeight() / 2));
        masker.setSeeds(seeds);

        // preparing masks
        Beatmup.Bitmap mask = new Beatmup.Bitmap(drawingTask.getContext(), heart.getWidth(), heart.getHeight(), PixelFormat.BinaryMask);
        mask.zero();
        masker.setOutput(mask);
        masker.execute();
        Rectangle maskCrop2 = new Rectangle();
        Beatmup.Bitmap mask2 = masker.optimizeMask(maskCrop2);

        mask = new Beatmup.Bitmap(drawingTask.getContext(), heart.getWidth(), heart.getHeight(), PixelFormat.QuaternaryMask);
        mask.zero();
        masker.setOutput(mask);
        masker.execute();
        Rectangle maskCrop4 = new Rectangle();
        Beatmup.Bitmap mask4 = masker.optimizeMask(maskCrop4);

        mask = new Beatmup.Bitmap(drawingTask.getContext(), heart.getWidth(), heart.getHeight(), PixelFormat.HexMask);
        mask.zero();
        masker.setOutput(mask);
        masker.execute();
        Rectangle maskCrop16 = new Rectangle();
        Beatmup.Bitmap mask16 = masker.optimizeMask(maskCrop16);

        // creating layers
        Scene.MaskedBitmapLayer l = scene.newMaskedBitmapLayer();
        l.setBitmap(heart);
        l.setMask(mask2);
        l.scale(0.55f);
        l.setCenterPosition(0.25f, 0.25f);
        l.setMaskTransform(new AffineMapping(maskCrop2));

        l = scene.newMaskedBitmapLayer();
        l.setBitmap(heart);
        l.setMask(mask4);
        l.scale(0.55f);
        l.setCenterPosition(0.75f, 0.25f);
        l.setMaskTransform(new AffineMapping(maskCrop4));

        l = scene.newMaskedBitmapLayer();
        l.setBitmap(heart);
        l.setMask(mask16);
        l.scale(0.6f);
        l.setCenterPosition(0.5f, 0.75f);
        l.setMaskTransform(new AffineMapping(maskCrop16));

        return scene;
    }
}

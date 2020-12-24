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

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import Beatmup.Android.Camera;
import Beatmup.Context;
import Beatmup.Exceptions.CoreException;
import Beatmup.NNets.DeserializedModel;
import Beatmup.NNets.ImageSampler;
import Beatmup.NNets.InferenceTask;
import Beatmup.NNets.Model;
import Beatmup.NNets.Softmax;
import Beatmup.Pipelining.Multitask;
import Beatmup.Pipelining.TaskHolder;
import Beatmup.Rendering.Scene;
import Beatmup.Task;
import Beatmup.Utils.ChunkCollection;
import Beatmup.Utils.ChunkAsset;
import Beatmup.Utils.Callback;

public class DogClassifier extends TestSample {
    private Multitask multitask;
    private StringBuilder report;

    /**
     * Classifier record: class label and its predicted probability
     */
    private class Record {
        private String label;
        private float probability;

        public Record(String label) {
            this.label = label;
            this.probability = 0;
        }

        void set(String label, float probability) {
            this.label = label;
            this.probability = probability;
        }

        float getProbability() {
            return probability;
        }

        String getLabel() {
            return label;
        }
    }

    @Override
    public String getCaption() {
        return "Dog classifier";
    }

    @Override
    public String getDescription() {
        return "Runs a neural net-based dog breed classifier using camera image captured in real time";
    }

    @Override
    public Scene designScene(Task drawingTask, Activity app, Camera camera, String extFile) throws IOException, CoreException {
        Context context = drawingTask.getContext();

        // get model data
        ChunkCollection chunks = new ChunkAsset(app.getAssets(), "dog_classifier.chunks");

        // build model; by convention, the chunk with empty id ("") contains the serialized model
        String modelCode = chunks.read("");
        Model model = new DeserializedModel(context, modelCode);

        // prepare inference task
        InferenceTask inference = new InferenceTask(context, model, chunks);
        inference.connect(camera.getImage(), "input", 0);

        // prepare list of labels and probabilities
        final Softmax softmax = Softmax.fromModel(model, "softmax");
        final ArrayList<Record> records = new ArrayList<>();
        final String[] labels = chunks.read("labels").split("\n");
        final Comparator<Record> comparator = new Comparator<Record>() {
            @Override
            public int compare(Record record1, Record record2) {
                return Float.compare(record2.getProbability(), record1.getProbability());
            }
        };
        for (String label : labels)
            records.add(new Record(label));

        // apply rotation to the input texture according to the camera and display orientations
        int orientation = camera.getOrientation(app.getWindowManager().getDefaultDisplay());
        ImageSampler imageSampler = ImageSampler.fromModel(model, "input");
        imageSampler.setRotation(orientation / 90);

        // setup a simple scene to draw the camera frame on screen
        Scene scene = new Scene();
        Scene.BitmapLayer layer = scene.newBitmapLayer();
        layer.setBitmap(camera.getImage());
        layer.rotate(orientation);
        layer.scale(0.9f);
        layer.setCenterPosition(0.5f, 0.5f);

        // setup a 3-stage multitask:
        multitask = new Multitask(context);
        //  (1) run inference
        final TaskHolder inferenceHolder = multitask.addTask(inference, Multitask.RepetitionPolicy.REPEAT_ALWAYS);
        //  (2) draw the camera frame on screen
        multitask.addTask(drawingTask, Multitask.RepetitionPolicy.REPEAT_ALWAYS);
        //  (3) write out the predicted probabilities in a CallbackTask
        multitask.addTask(new Callback(context) {
            public void run() {
                // fill the list of records
                float[] proba = softmax.getProbabilities();
                for (int i = 0; i < proba.length; ++i)
                    records.get(i).set(labels[i], proba[i]);

                // sort the list
                Collections.sort(records, comparator);

                // construct a string report: FPS + top 5 classes if their probability is high enough
                report = new StringBuilder(String.format("%.2f FPS", 1000 / inferenceHolder.getRunTime()));
                for (int i = 0; i < 5 && records.get(i).getProbability() >= 0.2; ++i) {
                    Record record = records.get(i);
                    report.append(String.format("\n%s: %.2f%%", record.getLabel(), 100 * record.getProbability()));
                }
            }
        }, Multitask.RepetitionPolicy.REPEAT_ALWAYS);
        multitask.measure();

        return scene;
    }

    @Override
    public boolean usesCamera() {
        return true;
    }

    @Override
    public Task getDrawingTask() {
        return multitask;
    }

    @Override
    public String getRuntimeInfo() {
        return report != null ? report.toString() : "";
    }
}

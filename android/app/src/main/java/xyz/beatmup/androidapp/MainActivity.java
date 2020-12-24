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

package xyz.beatmup.androidapp;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.hardware.camera2.CameraAccessException;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import Beatmup.Android.Camera;
import Beatmup.Android.Context;
import Beatmup.Android.Bitmap;
import Beatmup.Task;
import Beatmup.Visual.Android.BasicDisplay;
import Beatmup.Visual.Android.Display;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Rendering.Scene;
import Beatmup.Rendering.SceneRenderer;
import Beatmup.Visual.Animator;
import Beatmup.Visual.GestureListener;
import xyz.beatmup.androidapp.samples.BasicCameraUse;
import xyz.beatmup.androidapp.samples.BasicRendering;
import xyz.beatmup.androidapp.samples.DogClassifier;
import xyz.beatmup.androidapp.samples.HarmonicPlayback;
import xyz.beatmup.androidapp.samples.MultitaskRendering;
import xyz.beatmup.androidapp.samples.OptimizedMaskFromBitmap;
import xyz.beatmup.androidapp.samples.RecursiveSubscene;
import xyz.beatmup.androidapp.samples.TestSample;
import xyz.beatmup.androidapp.samples.UpsamplingConvnet;
import xyz.beatmup.androidapp.samples.UpsamplingConvnetOnCamera;
import xyz.beatmup.androidapp.samples.VideoDecoding;
import xyz.beatmup.androidapp.samples.WavFilePlayback;


public class MainActivity extends Activity {
    SceneRenderer renderer;
    Task drawingTask;
    Context context;
    TestSample currentTest;
    TestSample testToStart = null;               // a test to be started when activity is created
    String selectedExternalFile = null;          // external file path got in onActivityResult
    Camera camera;
    Dialog testSamplesSelectionDialog;
    int drawingJob;
    boolean activityStarted = false;


    /**
     * List of test samples
     */
    final TestSample[] TEST_SAMPLES = new TestSample[] {
            new BasicRendering(),
            new OptimizedMaskFromBitmap(),
            new RecursiveSubscene(),
            new BasicCameraUse(),
            new VideoDecoding(this),
            new UpsamplingConvnet(),
            new UpsamplingConvnetOnCamera(),
            new MultitaskRendering(),
            new HarmonicPlayback(),
            new WavFilePlayback(),
            new DogClassifier()
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // init context with two thread pools
        context = new Context(2);

        // shape the secondary pool to a single thread
        context.limitWorkerCount(1, 1);

        // setting up a renderer
        renderer = new SceneRenderer(context);
        renderer.setOutputMapping(SceneRenderer.OutputMapping.FIT_WIDTH);
        renderer.setOutputReferenceWidth(1000);
        renderer.setOutputPixelsFetching(true);
        try {
            renderer.setBackground(Bitmap.decodeStream(context, getAssets().open("bg.bmp")));
        } catch (IOException e) {
            e.printStackTrace();
        }

        // ask for permissions
        ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.CAMERA, Manifest.permission.READ_EXTERNAL_STORAGE}, 0);

        // init display
        final Display display = (Display) findViewById(R.id.display);
        display.setRenderer(renderer);
        display.setGestureListener(new GestureListener(0.05f) {
            Scene.Layer pickedLayer;

            @Override
            public void onTap(float x, float y) {
                Scene.Layer tappedLayer = renderer.pickLayer(x, y, false);
                if (tappedLayer != null)
                    currentTest.onTap(tappedLayer);
            }

            @Override
            public void onFirstTouch(float x, float y) {
                if (renderer.getScene() == null)
                    return;
                pickedLayer = renderer.pickLayer(x, y, false);
                if (pickedLayer != null)
                    setGesture(pickedLayer.getTransform(), 0.2f, 5.0f);
            }

            @Override
            public boolean onGesture(AffineMapping gesture) {
                if (pickedLayer != null) {
                    pickedLayer.setTransform(gesture);
                    currentTest.onGesture(pickedLayer, gesture);
                }
                return true;
            }

            @Override
            public void onRelease(AffineMapping gesture) {
                if (pickedLayer != null) {
                    pickedLayer.setTransform(gesture);
                }
            }

            @Override
            public void onHold(float x, float y) {
                Log.i(this.getClass().getName(), "Hold event!");
            }
        });

        display.getGestureListener().enableHoldEvent(Animator.getInstance());
        display.addBindingListener(new BasicDisplay.OnBindingListener() {
            @Override
            public void beforeBinding(boolean valid) {
                if (drawingTask != null)
                    context.abortJob(drawingJob);
            }

            @Override
            public void afterBinding(boolean valid) { }
        });

        // setup test sample selection dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View listing = getLayoutInflater().inflate(R.layout.test_samples_dialog, null);
        final TestSampleClickListener testSampleClickListener = new TestSampleClickListener();

        for (int i = 0; i < TEST_SAMPLES.length; ++i) {
            View entry = getLayoutInflater().inflate(R.layout.test_samples_dialog_entry, null);
            entry.setTag(i);
            TestSample sample = TEST_SAMPLES[i];
            ((TextView) entry.findViewById(R.id.textSampleTitle)).setText(sample.getCaption());
            ((TextView) entry.findViewById(R.id.textSampleDescription)).setText(sample.getDescription());
            ((LinearLayout) listing.findViewById(R.id.layoutTestSamplesList)).addView(entry);
            entry.setOnClickListener(testSampleClickListener);
        }

        builder.setView(listing);
        builder.setTitle("Select a test sample");
        builder.setCancelable(false);

        findViewById(R.id.buttonShowTestList).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                context.abortJob(drawingJob);
                testSamplesSelectionDialog.show();
            }
        });

        // setup a timer updating a little piece of text on the top containing useful info from the current test
        final Handler textInfoUpdater = new Handler();
        textInfoUpdater.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (currentTest != null)
                    ((TextView) findViewById(R.id.textInfo)).setText(currentTest.getRuntimeInfo());
                textInfoUpdater.postDelayed(this,1000);
            }
        },1000);

        // start a test if already selected (may be set in onActivityResult)
        if (testToStart == null)
            testSamplesSelectionDialog = builder.show();
        else {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    runTest(testToStart, selectedExternalFile);
                    testToStart = null;
                }
            });
        }

        activityStarted = true;
    }


    private class TestSampleClickListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            final int testSampleIdx = (Integer)view.getTag();
            final TestSample selectedTest = TEST_SAMPLES[testSampleIdx];

            if (selectedTest.usesCamera() &&
                ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
            ) {
                Toast.makeText(MainActivity.this, "No camera access granted, cannot run this test sample.", Toast.LENGTH_LONG).show();
                return;
            }

            if (selectedTest.usesExternalFile() != null &&
                ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
            ) {
                Toast.makeText(MainActivity.this, "No storage access granted, cannot run this test sample.", Toast.LENGTH_LONG).show();
                return;
            }

            if (selectedTest.usesExternalFile() == null)
                runTest(selectedTest, null);
            else {
                Intent intent = new Intent()
                        .setType(selectedTest.usesExternalFile())
                        .setAction(Intent.ACTION_GET_CONTENT)
                        .addCategory(Intent.CATEGORY_OPENABLE);
                testSamplesSelectionDialog.hide();
                startActivityForResult(Intent.createChooser(intent, "Select a file"), testSampleIdx);
            }
        }
    }


    @Override
    protected void onPause() {
        if (currentTest != null)
            currentTest.stop();
        super.onPause();
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        for (int i = 0; i < permissions.length; ++i)
            if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "Some examples will not run due to denied permissions.", Toast.LENGTH_LONG).show();
                break;
            }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (data == null || data.getData() == null) {
            testSamplesSelectionDialog.show();
            return;
        }

        // get URI
        Uri uri = data.getData();
        TestSample testSample = null;

        // try to reach local file first
        try (Cursor cursor = getApplicationContext().getContentResolver().query(uri, null, null, null, null)) {
            int columnIndex = cursor.getColumnIndexOrThrow(MediaStore.Files.FileColumns.DATA);
            cursor.moveToFirst();
            selectedExternalFile = cursor.getString(columnIndex);
            testSample = TEST_SAMPLES[requestCode];
        }
        catch (IllegalArgumentException ex) {
            ex.printStackTrace();
        }

        // if does not work, try to copy it to local storage
        if (testSample == null)
            try {
                InputStream input = getContentResolver().openInputStream(uri);
                File copy = new File(getFilesDir(), "copy.tmp");
                selectedExternalFile = copy.getAbsolutePath();
                OutputStream outputStream = new FileOutputStream(selectedExternalFile);
                byte[] buffer = new byte[1024 * 64];
                int length = 0;
                while ((length = input.read(buffer)) > 0)
                    outputStream.write(buffer,0, length);
                outputStream.close();
                input.close();
                testSample = TEST_SAMPLES[requestCode];
            } catch (Exception e) {
                e.printStackTrace();
                testSamplesSelectionDialog.show();
                Toast.makeText(MainActivity.this, "Cannot access the chosen file. Select another one.", Toast.LENGTH_LONG).show();
                return;
            }

        if (activityStarted && testSample != null)
            runTest(testSample, selectedExternalFile);
        else
            testToStart = testSample;
    }


    /**
     * Stops the ongoing test if any, prepares and starts another one. Shows a temporizing dialog.
     * @param test          The new test
     * @param extFile       An external file selected by the user if needed for the test
     */
    private void runTest(final TestSample test, final String extFile) {
        BackgroundAction.run(testSamplesSelectionDialog, "Preparing test sample...",
                new Runnable() {
                    @Override
                    public void run() {
                        // close everything
                        context.abortJob(drawingJob);
                        context.recycleGPUGarbage();
                        renderer.resetOutput();
                        if (currentTest != null) {
                            currentTest.stop();
                            if (currentTest.usesCamera())
                                camera.close();
                        }

                        // set new test as current
                        currentTest = test;

                        // open camera if required by the selected test sample
                        if (currentTest.usesCamera()) {
                            if (camera == null)
                                try {
                                    camera = new Camera(context, MainActivity.this);
                                    camera.chooseResolution(385,385, Camera.ResolutionSelectionPolicy.SMALLEST_COVERAGE);
                                    Log.i("Beatmup", "Camera resolution: "
                                            + Integer.toString(camera.getResolution().getWidth()) + "x"
                                            + Integer.toString(camera.getResolution().getHeight()));
                                } catch (CameraAccessException e) {
                                    e.printStackTrace();
                                }
                            camera.open();
                        }

                        // construct new scene and start drawing
                        try {
                            Scene scene = currentTest.designScene(
                                    renderer, MainActivity.this,
                                    currentTest.usesCamera() ? camera : null,
                                    extFile
                            );
                            renderer.setScene(scene);
                            drawingTask = currentTest.getDrawingTask();
                            if (drawingTask == null)
                                drawingTask = renderer;

                            // start drawing
                            drawingJob = context.submitPersistentTask(drawingTask);
                        } catch (Throwable ex) {
                            ex.printStackTrace();

                            // report the error out loud
                            final String error = ex.getClass().getSimpleName();
                            MainActivity.this.runOnUiThread(
                                    new Runnable() {
                                        @Override
                                        public void run() {
                                            Toast.makeText(MainActivity.this, "Something went wrong (" + error + ")", Toast.LENGTH_LONG).show();
                                            testSamplesSelectionDialog.show();
                                        }
                                    }
                            );
                        }
                    }
                });
    }
}

package xyz.beatmup.android.app;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.params.Face;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;

import Beatmup.Android.Camera;
import Beatmup.Android.Context;
import Beatmup.Audio.Playback;
import Beatmup.Audio.SampleFormat;
import Beatmup.Audio.Source.Harmonic;
import Beatmup.Exceptions.PlaybackException;
import Beatmup.Geometry.IntPoint;
import Beatmup.Android.Bitmap;
import Beatmup.Imaging.Color;
import Beatmup.Imaging.ColorMatrix;
import Beatmup.Imaging.FloodFill;
import Beatmup.NNets.GPUBenchmark;
import Beatmup.Pipelining.Multitask;
import Beatmup.Pipelining.TaskHolder;
import Beatmup.Shading.Shader;
import Beatmup.Shading.ShaderApplicator;
import Beatmup.Task;
import Beatmup.Visual.Android.Display;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Geometry.Rectangle;
import Beatmup.Imaging.PixelFormat;
import Beatmup.Rendering.Scene;
import Beatmup.Rendering.SceneRenderer;
import Beatmup.Visual.Animator;
import Beatmup.Visual.GestureListener;
import xyz.beatmup.android.app.samples.BasicRendering;
import xyz.beatmup.android.app.samples.TestSample;


public class MainActivity extends Activity {
    Bitmap output;
    SceneRenderer renderer;
    Task drawingTask;
    Context context;
    TestSample currentTest;
    Camera camera;
    Dialog testSamplesSelectionDialog;

    final TestSample[] TEST_SAMPLES = new TestSample[] {

            new TestSample() {
                @Override
                public String getCaption() {
                    return "GPU benchmak";
                }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    //GPUBenchmark bench = new GPUBenchmark(context);
                    Log.i("Beatmup internal dir", Environment.getExternalStorageDirectory().getAbsolutePath());
                    GPUBenchmark.test(
                            Bitmap.decodeStream(context, getAssets().open("test1.png")),
                            context
                    );
                    //bench.execute();
                    //Log.i("GPU BENCHMARK", String.format("Error: %f, score: %.2f", bench.getError(), bench.getScore() * 1e-6));
                    return null;
                }
            },

            new BasicRendering(),

            new TestSample() {
                @Override
                public String getCaption() {
                    return "Optimized mask from bitmap";
                }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    Scene scene = new Scene();

                    Beatmup.Bitmap heart = Bitmap.decodeStream(context, getAssets().open("heart.png"));

                    // preparing masker
                    FloodFill masker = new FloodFill(context);
                    masker.setInput(heart);
                    masker.setBorderPostprocessing(FloodFill.BorderMorphology.DILATE, 1, 40);
                    ArrayList<IntPoint> seeds = new ArrayList<>();
                    seeds.add(new IntPoint(heart.getWidth() / 2, heart.getHeight() / 2));
                    masker.setSeeds(seeds);

                    // preparing masks
                    Beatmup.Bitmap mask = heart.clone(PixelFormat.BinaryMask);
                    mask.zero();
                    masker.setOutput(mask);
                    masker.execute();
                    Rectangle maskCrop2 = new Rectangle();
                    Beatmup.Bitmap mask2 = masker.optimizeMask(maskCrop2);

                    mask = heart.clone(PixelFormat.QuaternaryMask);
                    mask.zero();
                    masker.setOutput(mask);
                    masker.execute();
                    Rectangle maskCrop4 = new Rectangle();
                    Beatmup.Bitmap mask4 = masker.optimizeMask(maskCrop4);

                    mask = heart.clone(PixelFormat.HexMask);
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
            },


            new TestSample() {
                @Override
                public String getCaption() {
                    return "Shaped bitmap layers";
                }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    Scene scene = new Scene();
                    Bitmap bitmap = Bitmap.decodeStream(context, getAssets().open("kitten.jpg"));

                    Scene.ShapedBitmapLayer layer = scene.newShapedBitmapLayer();
                    layer.setBitmap(bitmap);
                    layer.scale(0.45f);
                    layer.setCornerRadius(50);
                    layer.setBorderWidth(10);
                    layer.setSlopeWidth(100);
                    layer.setCenterPosition(0.25f, 0.25f);

                    layer = scene.newShapedBitmapLayer();
                    layer.setBitmap(bitmap);
                    layer.scale(0.45f);
                    layer.rotate(30);
                    layer.setImageTransform(layer.getImageTransform().rotateAround(0.5f, 0.5f, -30));
                    layer.setCornerRadius(10);
                    layer.setBorderWidth(10);
                    layer.setSlopeWidth(10);
                    layer.setCenterPosition(0.75f, 0.25f);
                    layer.setModulationColor(Color.ORANGE);

                    layer = scene.newShapedBitmapLayer();
                    layer.setBitmap(bitmap);
                    layer.scale(0.45f);
                    layer.setMaskTransform(new AffineMapping().rotateAround(0.5f, 0.5f, 30));
                    layer.setCornerRadius(10);
                    layer.setBorderWidth(10);
                    layer.setSlopeWidth(10);
                    layer.setCenterPosition(0.25f, 0.75f);
                    layer.setModulationColor(Color.BLUE);

                    layer = scene.newShapedBitmapLayer();
                    layer.setBitmap(bitmap);
                    layer.scale(0.45f);
                    layer.setCornerRadius(100);
                    layer.setInPixels(true);
                    layer.setSlopeWidth(1);
                    layer.setCenterPosition(0.75f, 0.75f);
                    layer.setTransparency(0.5f);

                    return scene;
                }

            },

            new TestSample() {
                @Override
                public String getCaption() {
                    return "Subscene";
                }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    Scene scene = new Scene();
                    Bitmap bitmap = Bitmap.decodeStream(context, getAssets().open("kitten.jpg"));

                    Scene.BitmapLayer layer = scene.newBitmapLayer();
                    layer.setBitmap(bitmap);

                    Scene.SceneLayer subscene = scene.newSceneLayer(scene);
                    AffineMapping mapping = new AffineMapping();
                    mapping.rotateAround(0.5f, 0.5f, 5);
                    mapping.scale(0.95f);
                    subscene.setTransform(mapping);
                    subscene.setCenterPosition(0.5f, 0.5f);
                    return scene;
                }

            },


            new TestSample() {
                @Override
                public String getCaption() {
                    return "Camera";
                }

                public String getDescription() {
                    return "Camera usage example. A layer displaying the camera image and a shader layer applying a fancy transformation to it.";
                }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    Scene scene = new Scene();

                    Scene.BitmapLayer layer1 = scene.newBitmapLayer();
                    layer1.setImageSource(Scene.BitmapLayer.ImageSource.CAMERA);
                    layer1.rotate(90);
                    layer1.scale(0.5f);
                    layer1.setCenterPosition(0.25f, 0.25f);

                    Scene.ShadedBitmapLayer layer2 = scene.newShadedBitmapLayer();
                    layer2.rotate(90);
                    layer2.setImageSource(Scene.BitmapLayer.ImageSource.CAMERA);
                    layer2.scale(0.5f);
                    layer2.setCenterPosition(0.75f, 0.75f);

                    layer2.setShader(new Shader(context));
                    ColorMatrix matrix = new ColorMatrix();
                    matrix.setColorInversion(Color.GREEN, 1, 1);
                    layer2.getShader().setColorMatrix("transform", matrix);

                    layer2.getShader().setSourceCode(
                            "beatmupInputImage image;" +
                            "varying mediump vec2 texCoord;" +
                            "uniform mediump mat4 transform;" +
                            "void main() {" +
                            "   highp vec2 xy = vec2(texCoord.x, min(texCoord.y, 1.0-texCoord.y));" +
                            "   gl_FragColor.rgba = transform * texture2D(image, xy).rgba;" +
                            "}"
                    );
                    return scene;
                }

                @Override
                public boolean usesCamera() {
                    return true;
                }
            },

            new TestSample() {
                private Multitask multitask;
                private TaskHolder shaderApplication;

                @Override
                public String getCaption() {
                    return "Multitask";
                }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    Bitmap bitmap = Bitmap.decodeStream(context, getAssets().open("kitten.jpg"));

                    ShaderApplicator applicator = new ShaderApplicator(context);
                    applicator.setInput(bitmap);
                    applicator.setOutput(Beatmup.Bitmap.createEmpty(bitmap));
                    applicator.setShader(new Shader(context));
                    ColorMatrix matrix = new ColorMatrix();
                    matrix.setColorInversion(Color.GREEN, 1, 1);
                    applicator.getShader().setColorMatrix("transform", matrix);
                    applicator.getShader().setSourceCode(
                            "beatmupInputImage image;" +
                            "varying mediump vec2 texCoord;" +
                            "uniform mediump mat4 transform;" +
                            "uniform highp float dx;" +
                            "uniform highp float dy;" +
                            "void main() {" +
                            "   highp vec3 clr = texture2D(image, texCoord.xy).rgb;" +
                            "   gl_FragColor.rgba = transform * vec4(clr, 1.0);" +
                            "}"
                    );
                    applicator.getShader().setFloat("dx", 1.0f/bitmap.getWidth());
                    applicator.getShader().setFloat("dy", 1.0f/bitmap.getHeight());

                    Scene scene = new Scene();
                    Scene.ShapedBitmapLayer shaped = scene.newShapedBitmapLayer();
                    shaped.setBitmap(bitmap);
                    shaped.scale(0.45f);
                    shaped.setCornerRadius(10);
                    shaped.setBorderWidth(10);
                    shaped.setSlopeWidth(10);
                    shaped.setCenterPosition(0.75f, 0.25f);
                    shaped.setModulationColor(Color.ORANGE);

                    Scene.BitmapLayer bitmapLayer = scene.newBitmapLayer();
                    bitmapLayer.setBitmap(applicator.getOutput());
                    bitmapLayer.scale(0.5f);
                    bitmapLayer.setCenterPosition(0.5f, 0.5f);
                    bitmapLayer.setName("Switch color");

                    shaped = scene.newShapedBitmapLayer();
                    shaped.setBitmap(bitmap);
                    shaped.scale(0.45f);
                    shaped.setCornerRadius(10);
                    shaped.setBorderWidth(10);
                    shaped.setSlopeWidth(10);
                    shaped.setCenterPosition(0.25f, 0.75f);
                    shaped.setModulationColor(Color.BLUE);

                    multitask = new Multitask(context);
                    shaderApplication = multitask.addTask(applicator, Multitask.RepetitionPolicy.REPEAT_UPDATE);
                    multitask.addTask(renderer);
                    multitask.measure();

                    return scene;
                }

                @Override
                public Task getDrawingTask() {
                    return multitask;
                }

                @Override
                public void onTap(Scene.Layer layer) {
                    if (layer.getName().equals("Switch color")) {
                        ColorMatrix matrix = new ColorMatrix();
                        matrix.setColorInversion(
                                Color.byHue((float) Math.random() * 360.0f), 1, 1
                        );
                        ((ShaderApplicator)shaderApplication.getTask()).getShader().setColorMatrix("transform", matrix);
                        multitask.setRepetitionPolicy(shaderApplication, Multitask.RepetitionPolicy.REPEAT_UPDATE);
                    }
                }
            },


            new TestSample() {
                Playback playback;
                Harmonic harmonic;

                @Override
                public String getCaption() { return "Audio playback: harmonic"; }

                @Override
                public Scene designScene(Beatmup.Context context, Activity app) throws IOException {
                    harmonic = new Harmonic();
                    playback = new Playback(context);
                    playback.setSource(harmonic);
                    try {
                        playback.configure(SampleFormat.Int16, 44100, 1, 2, 64);
                        playback.start();
                        context.submitPersistentTask(playback);
                    } catch (PlaybackException e) {
                        e.printStackTrace();
                    }

                    Scene scene = new Scene();
                    Bitmap androidBitmap = Bitmap.decodeStream(context, getAssets().open("kitten.jpg"));

                    Beatmup.Bitmap bitmap1 = context.copyBitmap(androidBitmap, PixelFormat.SingleByte);
                    Scene.BitmapLayer layer = scene.newBitmapLayer();
                    layer.setBitmap(bitmap1);
                    layer.setPosition(0.1f, 0.1f);
                    layer.scale(0.25f);
                    layer.rotate(10);
                    return scene;
                }

                @Override
                public void onGesture(Scene.Layer layer, AffineMapping mapping) {
                    harmonic.setFrequency(layer.getX() * 2000);
                    harmonic.setAmplitude(layer.getY() * 0.5f);
                }
            }
    };

    private class TestListAdapter extends ArrayAdapter<TestSample> {

        TestListAdapter(Activity activity) {
            super(activity, android.R.layout.simple_list_item_1, TEST_SAMPLES);
        }

        @Override
        public boolean isEnabled(int position) {
            return !TEST_SAMPLES[position].usesCamera() ||
                    (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.CAMERA)
                            == PackageManager.PERMISSION_GRANTED);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // init context
        context = new Context(2, getFilesDir());
        context.limitWorkerCount(1, 1);

        // setting up a renderer
        renderer = new SceneRenderer(context);
        renderer.setOutputMapping(SceneRenderer.OutputMapping.FIT_WIDTH);
        renderer.setOutputReferenceWidth(1000);
        renderer.setOutputPixelsFetching(true);
        try {
            renderer.setBackground(
                    Bitmap.decodeStream(context, getAssets().open("bg.png"))
            );
        } catch (IOException e) {
            e.printStackTrace();
        }

        // camera
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                == PackageManager.PERMISSION_DENIED)
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.CAMERA}, 0);
        else
            setupCamera();

        // init display
        final Display display = (Display) findViewById(R.id.display);
        display.setRenderer(renderer);
        display.setGestureListener(new GestureListener(0.05f) {
            Scene.Layer pickedLayer;

            @Override
            public void onTap(float x, float y) {
                float ar = (float)display.getWidth() / display.getHeight();
                Scene.Layer tappedLayer = renderer.pickLayer(x, y, true);
                if (tappedLayer != null)
                    currentTest.onTap(tappedLayer);
            }

            @Override
            public void onFirstTouch(float x, float y) {
                if (renderer.getScene() == null)
                    return;
                pickedLayer = renderer.pickLayer(x, y, true);
                if (pickedLayer != null)
                    setGesture(pickedLayer.getTransform(), 0.2f, 5.0f);
            }

            @Override
            public boolean onGesture(AffineMapping gesture) {
                if (pickedLayer != null) {
                    pickedLayer.setTransform(gesture);
                    context.repeatTask(drawingTask, true);
                    currentTest.onGesture(pickedLayer, gesture);
                }
                return true;
            }

            @Override
            public void onRelease(AffineMapping gesture) {
                if (pickedLayer != null) {
                    pickedLayer.setTransform(gesture);
                    context.repeatTask(drawingTask, true);
                }
            }

            @Override
            public void onHold(float x, float y) {
                Log.i(this.getClass().getName(), "Hold event!");
            }
        });
        display.getGestureListener().enableHoldEvent(Animator.getInstance());

        // setup test sample selection dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View listing = getLayoutInflater().inflate(R.layout.test_samples_dialog, null);
        final TestSampleClickListener testSampleClickListener = new TestSampleClickListener();

        for (TestSample sample : TEST_SAMPLES) {
            View entry = getLayoutInflater().inflate(R.layout.test_samples_dialog_entry, null);
            entry.setTag(sample);
            ((TextView) entry.findViewById(R.id.textSampleTitle)).setText(sample.getCaption());
            ((TextView) entry.findViewById(R.id.textSampleDescription)).setText(sample.getDescription());
            ((LinearLayout) listing.findViewById(R.id.layoutTestSamplesList)).addView(entry);
            entry.setOnClickListener(testSampleClickListener);
        }

        builder.setView(listing)
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {

                    }
                });
        builder.setTitle("Select a test sample");
        testSamplesSelectionDialog = builder.show();

        findViewById(R.id.buttonShowTestList).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                testSamplesSelectionDialog.show();
            }
        });
    }


    private class TestSampleClickListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            testSamplesSelectionDialog.hide();
            context.recycleGPUGarbage();
            if (view.getTag() != currentTest) {
                if (currentTest != null && currentTest.usesCamera())
                    camera.close();
                currentTest = (TestSample)view.getTag();

                try {
                    renderer.setScene(currentTest.designScene(context, MainActivity.this));
                    if (currentTest.getDrawingTask() != null)
                        drawingTask = currentTest.getDrawingTask();
                    else
                        drawingTask = renderer;
                } catch (Exception e) {
                    e.printStackTrace();
                }

                if (currentTest.usesCamera()) {
                    camera.open();
                    camera.setCallback(new Camera.Callback() {
                        public void onAccessError(Camera camera) {}
                        public void onConfigureFailed(Camera camera) {}
                        public void onCameraError(Camera camera, int code) {}
                        public void onCameraOpened(Camera camera) {}
                        public void onCameraClosed(Camera camera) {}
                        public void onFacesDetected(Camera camera, Face[] faces) {}
                        public void onFrameReceived(Camera camera) {
                            context.repeatTask(drawingTask, false);
                        }
                    });
                }
            }
            renderer.resetOutput();

            float time = context.performTask(drawingTask);
            ((TextView) findViewById(R.id.textInfo)).setText(String.format("%.2f ms", Float.valueOf(time)));
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            setupCamera();
        } else {
            Toast.makeText(this, "Some examples will not run without camera permissions.", Toast.LENGTH_LONG).show();
        }
    }

    public void makeSnapshot(View view) {
        output = Bitmap.createColorBitmap(context, view.getWidth(), view.getHeight());
        renderer.setOutput(output);
        float time = renderer.render();
        ((TextView) findViewById(R.id.textInfo)).setText(String.format("%.2f ms", Float.valueOf(time)));
        ((ImageView) view).setImageBitmap(output.getBitmap());
        renderer.resetOutput();
    }


    private void setupCamera() {
        try {
            camera = new Camera(context, this);
            camera.chooseResolution(224,224, Camera.ResolutionSelectionPolicy.SMALLEST_COVERAGE);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}

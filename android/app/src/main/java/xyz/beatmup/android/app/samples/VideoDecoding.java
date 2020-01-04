package xyz.beatmup.android.app.samples;

import android.app.Activity;

import java.io.IOException;

import Beatmup.Android.Camera;
import Beatmup.Android.Decoder;
import Beatmup.Android.ExternalBitmap;
import Beatmup.Rendering.Scene;
import Beatmup.Rendering.SceneRenderer;
import Beatmup.Task;
import xyz.beatmup.android.app.MainActivity;

public class VideoDecoding extends TestSample {
    Decoder decoder1, decoder2;
    MainActivity activity;

    public VideoDecoding(MainActivity activity) {
        this.activity = activity;
    }

    @Override
    public String getCaption() {
        return "Video decoding";
    }

    @Override
    public String getDescription() {
        return "Basic video decoder usage.";
    }

    @Override
    public Scene designScene(final Task drawingTask, Activity app, Camera camera) throws IOException {
        decoder1 = new Decoder(drawingTask.getContext());
        decoder1.open(app.getAssets().openFd("VID_20190608_200239.mp4"));
        Decoder.VideoTrack videoTrack1 = decoder1.selectDefaultVideoTrack();
        videoTrack1.changeBlockingPolicy(Decoder.BlockingPolicy.PLAYBACK);
        videoTrack1.setFrameAvailableCallback(new Decoder.FrameAvailableCallback() {
            @Override
            public void onFrameAvailable() {
                drawingTask.execute();
            }
        });

        decoder2 = new Decoder(drawingTask.getContext());
        decoder2.open(app.getAssets().openFd("V_20160714_222043.mp4"));
        Decoder.VideoTrack videoTrack2 = decoder2.selectDefaultVideoTrack();
        videoTrack1.changeBlockingPolicy(Decoder.BlockingPolicy.PLAYBACK);
        videoTrack2.setFrameAvailableCallback(new Decoder.FrameAvailableCallback() {
            @Override
            public void onFrameAvailable() {
                drawingTask.execute();
            }
        });

        Scene scene = new Scene();
        Scene.BitmapLayer layer1 = scene.newBitmapLayer();
        layer1.setBitmap(videoTrack1.getFrame());
        layer1.scale(0.5f);
        layer1.setCenterPosition(0.5f, 0.25f);

        Scene.BitmapLayer layer2 = scene.newBitmapLayer();
        layer2.setBitmap(videoTrack2.getFrame());
        layer2.scale(0.5f);
        layer2.setCenterPosition(0.5f, 0.75f);

        decoder1.play();
        decoder2.play();

        return scene;
    }
}

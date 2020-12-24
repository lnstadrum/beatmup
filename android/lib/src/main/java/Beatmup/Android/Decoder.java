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

package Beatmup.Android;

import android.content.res.AssetFileDescriptor;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.util.ArrayMap;
import android.util.Log;
import android.view.Surface;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Map;

import Beatmup.Utils.SpeedLimiter;

/**
 * Android video decoder API
 */
public class Decoder {
    /**
     * Controls decoder realtime behavior.
     */
    public enum BlockingPolicy {
        /**
         * Decoding waits until every frame is processed in a respective callback. It guarantees
         * that all the frames are processed even though it may be slower than the playback speed.
         * This is generally not realtime (too slow or too fast for a realtime playback).
         */
        PROCESSING,

        /**
         * Frames are flushed into the output image without controlling whether a frame is consumed
         * or not. Some frames may not appear during the rendering if the latter is too slow. This
         * allows to keep at realtime pace sacrificing some frames. If the decoding process is too
         * fast, it slows down to match playback speed.
         */
        PLAYBACK
    }


    private static final String LOG_TAG = "Beatmup decoder";

    /**
     * Sequence of samples or frames in time in the encoded content
     */
    public static abstract class Track {
        class TrackThread extends Thread {
            boolean running, stop;

            TrackThread() {
                stop = false;
            }

            @Override
            public void run() {
                running = true;
                while (running && !Track.this.eos && !stop) {
                    Track.this.doDecode();
                }
                running = false;
            }

            void terminate() {
                stop = true;
                try {
                    join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        TrackThread decodingThread;
        boolean eos;

        Track(MediaFormat format) {
            decodingThread = null;
        }

        protected synchronized void play() {
            if (decodingThread != null)
                stop();
            eos = false;
            decodingThread = new TrackThread();
            decodingThread.start();
        }

        protected synchronized void stop() {
            if (decodingThread != null) {
                decodingThread.terminate();
                decodingThread = null;
            }
        }

        protected synchronized boolean isDecodingInProgress() {
            return  decodingThread != null && decodingThread.running;
        }

        /**
         * Provides input data entry point.
         * @return byte buffer expected to be filled with data to decoding.
         */
        protected abstract ByteBuffer getInputPacket();

        /**
         * Sends new input packet to be decoded.
         * @param size packet size
         * @param presentationTimestampUs packet presentation timestamp in microseconds
         */
        protected abstract void queueInputPacket(int size, long presentationTimestampUs);

        /**
         * Declares End of Stream on input (i.e., there are no more packets to decoding).
         */
        protected abstract void sendEos();

        protected abstract void doDecode();

        protected synchronized void resume() {
            notifyAll();
        }

        protected synchronized void pause() {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Implements a callback function invoked by Decoder when a new frame is available
     */
    public static abstract class FrameAvailableCallback {
        public abstract void onFrameAvailable();
    }

    /**
     * Video track
     */
    public static class VideoTrack extends Track {
        static final int
                INPUT_DEQUEUE_TIMEOUT_US = -1,
                OUTPUT_DEQUEUE_TIMEOUT_US = -1;

        Surface surface;                // output surface
        ExternalBitmap outputImage;     // output image
        MediaCodec decoder;
        int inputBufferId;
        MediaCodec.BufferInfo outputBufferInfo;
        int width, height;
        SpeedLimiter limiter;
        BlockingPolicy blockingPolicy;
        FrameAvailableCallback frameAvailableCallback;

        VideoTrack(Beatmup.Context engine, MediaFormat format) throws IOException {
            super(format);
            width = format.getInteger(MediaFormat.KEY_WIDTH);
            height = format.getInteger(MediaFormat.KEY_HEIGHT);
            limiter = new SpeedLimiter();
            blockingPolicy = BlockingPolicy.PLAYBACK;
            frameAvailableCallback = null;
            inputBufferId = -1;
            outputBufferInfo = new MediaCodec.BufferInfo();

            // configure output surface/texture
            outputImage = new ExternalBitmap(engine);
            surface = new Surface(outputImage.getSurfaceTexture());
            outputImage.getSurfaceTexture().setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
                @Override
                public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                    outputImage.notifyUpdate(width, height);
                    if (frameAvailableCallback != null) {
                        frameAvailableCallback.onFrameAvailable();
                        resume();
                    }
                }
            });

            // setup decoder
            decoder = MediaCodec.createDecoderByType(format.getString(MediaFormat.KEY_MIME));
            decoder.configure(format, surface, null, 0);
        }

        /**
         * @return a decoded image.
         */
        public ExternalBitmap getFrame() {
            return outputImage;
        }

        /**
         * Changes blocking policy.
         * The blocking policy cannot be changed if decoding is in progress (i.e., between start()
         * and stop() calls. An exception is thrown in this case.
         * @param policy new blocking policy
         */
        public synchronized void changeBlockingPolicy(BlockingPolicy policy) {
            if (isDecodingInProgress())
                throw new IllegalStateException("Cannot change blocking policy during the encoding process");
            this.blockingPolicy = policy;
        }

        /**
         * @return the current blocking policy.
         */
        public BlockingPolicy getBlockingPolicy() {
            return blockingPolicy;
        }

        /**
         * Sets frame available callback.
         * In processing mode (see {@link Decoder} constructor) the decoded frame texture given by getFrame() MUST BE PROCESSED within this callback, e.g. be rendered within a
         * Scene. Namely updateTexImage() needs to be called on its respective {@link SurfaceTexture}, otherwise the decoder stalls in a deadlock.
         * @param callback the callback.
         */
        public void setFrameAvailableCallback(FrameAvailableCallback callback) {
            this.frameAvailableCallback = callback;
        }

        @Override
        protected ByteBuffer getInputPacket() {
            if (inputBufferId  < 0)
                inputBufferId = decoder.dequeueInputBuffer(INPUT_DEQUEUE_TIMEOUT_US);
            return inputBufferId >= 0 ? decoder.getInputBuffer(inputBufferId) : null;
        }

        @Override
        protected void queueInputPacket(int size, long presentationTimestampUs) {
            decoder.queueInputBuffer(inputBufferId, 0, size, presentationTimestampUs, 0);
            inputBufferId = -1;
        }

        @Override
        protected void sendEos() {
            decoder.queueInputBuffer(inputBufferId, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
        }


        @Override
        protected synchronized void play() {
            decoder.start();
            eos = false;
            super.play();
        }

        @Override
        protected synchronized void stop() {
            super.stop();
            decoder.stop();
        }

        protected void doDecode() {
            int status = decoder.dequeueOutputBuffer(outputBufferInfo, OUTPUT_DEQUEUE_TIMEOUT_US);
            if (status == MediaCodec.INFO_TRY_AGAIN_LATER) {
                //
            }
            else if (status == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                // format change; update texture size
                MediaFormat format = decoder.getOutputFormat();
                width = format.getInteger(MediaFormat.KEY_WIDTH);
                height = format.getInteger(MediaFormat.KEY_HEIGHT);
            }
            else if (status == MediaCodec.BUFFER_FLAG_END_OF_STREAM) {
                //
            }
            else if (status < 0) {
                Log.e(LOG_TAG, String.format("Decoding error %d", status));
            }
            else {
                // a frame got
                if ((outputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0)
                    eos = true;
                // release the decoded data to client
                boolean doRender = outputBufferInfo.size != 0;
                decoder.releaseOutputBuffer(status, doRender);
                // control blocking
                if (blockingPolicy == BlockingPolicy.PROCESSING) {
                    if (doRender && frameAvailableCallback != null) {
                        pause();
                    }
                }
                else {
                    limiter.tick(outputBufferInfo.presentationTimeUs);
                }

            }
        }
    }

    private final Beatmup.Context engine;
    private Map<Integer, Track> tracks;
    private MediaExtractor extractor;
    private Thread extractingThread;
    private boolean running;

    /**
     * Instantiates decoder.
     * @param engine        Beatmup engine instance
     */
    public Decoder(Beatmup.Context engine) {
        this.engine = engine;
        tracks = new ArrayMap<>();
        running = false;
        extractingThread = new Thread(new Runnable() {
            @Override
            public void run() {
                doExtract();
            }
        });
    }

    /**
     * Opens an asset file to decode.
     * @param file the input file
     * @throws FileNotFoundException if ever the file cannot be accessed or read.
     */
    public void open(AssetFileDescriptor file) throws FileNotFoundException {
        if (!file.getFileDescriptor().valid())
            throw new FileNotFoundException(file.toString());

        extractor = new MediaExtractor();
        try {
            extractor.setDataSource(file);
        } catch (IOException e) {
            throw new FileNotFoundException(file.toString());
        }

        if (extractor.getTrackCount() == 0)
            throw new FileNotFoundException(file.toString());
    }

    /**
     * Opens a file to decode.
     * @param file the input file
     * @throws FileNotFoundException if ever the file cannot be accessed or read.
     */
    public void open(File file) throws FileNotFoundException {
        if (!file.canRead())
            throw new FileNotFoundException(file.toString());

        extractor = new MediaExtractor();
        try {
            extractor.setDataSource(file.getAbsolutePath());
        } catch (IOException e) {
            throw new FileNotFoundException(file.toString());
        }

        if (extractor.getTrackCount() == 0)
            throw new FileNotFoundException(file.toString());
    }

    /**
     * Looks for a default video track in the input.
     * @return a {@link VideoTrack} instance if there is a video track available in the input content, null otherwise.
     * @throws IOException if no decoder is available for the default video track.
     */
    public VideoTrack selectDefaultVideoTrack() throws IOException {
        // check if opened
        if (extractor == null)
            return null;

        // loop tracks
        int numTracks = extractor.getTrackCount();
        for (int i = 0; i < numTracks; i++) {
            MediaFormat format = extractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            // if this is a video track
            if (mime.startsWith("video/")) {
                // check if the track is already selected
                if (tracks.containsKey(i))
                    return (VideoTrack)tracks.get(i);

                // select the track
                extractor.selectTrack(i);
                VideoTrack track = new VideoTrack(engine, format);
                tracks.put(i, track);

                // return it
                return track;
            }
        }

        return null;
    }

    /**
     * Starts playing all the tracks.
     */
    public synchronized void play() {
        if (running)
            stop();
        for (Track t : tracks.values())
            t.play();
        running = true;
        extractingThread.start();
    }

    /**
     * Stops playing all the tracks.
     */
    public synchronized void stop() {
        for (Track t : tracks.values())
            t.stop();
        running = false;
        try {
            extractingThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    private void doExtract() {
        while (running) {
            int trackIndex = extractor.getSampleTrackIndex();
            if (trackIndex < 0)
                // EOS
                break;

            // get a buffer from MediaExtractor
            if (tracks.containsKey(trackIndex)) {
                Track track = tracks.get(trackIndex);
                ByteBuffer buffer = track.getInputPacket();
                if (buffer != null) {
                    int chunkSize = extractor.readSampleData(buffer, 0);
                    if (chunkSize > 0)
                        track.queueInputPacket(chunkSize, extractor.getSampleTime());
                    else
                        track.sendEos();
                }
                else
                    continue;
            }

            extractor.advance();
        }
    }
}

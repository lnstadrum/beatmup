package Beatmup.Media;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.Build;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;


import java.nio.ByteBuffer;

/**
 * Media decoder
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public abstract class AndroidMediaDecoder {

    private MediaExtractor extractor;
    private MediaCodec decoder;

    private int selectedTrack = -1;

    public static class InvalidFormat extends Exception {
        public InvalidFormat(String message) {
            super(message);
        }
    }

    public static class DecoderNotReady extends Exception {
        public DecoderNotReady(String message) {
            super(message);
        }
    }


    public AndroidMediaDecoder(String filename) throws IOException {
        File inputFile = new File(filename);
        if (!inputFile.canRead())
            throw new FileNotFoundException("Unable to read " + inputFile);
        extractor = new MediaExtractor();
        extractor.setDataSource(inputFile.toString());
    }


    public void selectTrack(int index) {
        int numTracks = extractor.getTrackCount();
        if (index < 0 || index >= numTracks)
            throw new IllegalArgumentException("Track index out of range");
        selectedTrack = index;
        extractor.selectTrack(index);
    }


    public void extract() throws InvalidFormat, DecoderNotReady, IOException {
        if (selectedTrack < 0)
            throw new DecoderNotReady("No track selected");

        // initialize decoder
        MediaFormat format = extractor.getTrackFormat(selectedTrack);
        try {
            if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                // since Lollipop it is preferred to pick up a codec by name, not by MIME type
                format.setString(MediaFormat.KEY_FRAME_RATE, null);
                decoder = MediaCodec.createByCodecName(
                        new MediaCodecList(MediaCodecList.REGULAR_CODECS).findDecoderForFormat(format)
                );
            } else
                decoder = MediaCodec.createDecoderByType(format.getString(MediaFormat.KEY_MIME));
        } catch (IllegalArgumentException ex) {
            throw new InvalidFormat("Unsupported format");
        }

        decoder.configure(format, null, null, 0);
        decoder.start();

        final int TIMEOUT_USEC = 10000;
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

        ByteBuffer[] inputBuffers = null, outputBuffers = null;
        if (android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            inputBuffers = decoder.getInputBuffers();
            outputBuffers = decoder.getOutputBuffers();
        }

        boolean outputDone = false;
        boolean inputDone = false;
        while (!outputDone) {

            // Feed more data to the decoder.
            if (!inputDone) {
                int inputBufIndex = decoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (inputBufIndex >= 0) {
                    // get input buffer
                    ByteBuffer inputBuf;
                    if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                        inputBuf = decoder.getInputBuffer(inputBufIndex);
                    else
                        inputBuf = inputBuffers[inputBufIndex];

                    // Read the sample data into the ByteBuffer.  This neither respects nor
                    // updates inputBuf's position, limit, etc.
                    int chunkSize = extractor.readSampleData(inputBuf, 0);
                    if (chunkSize < 0) {
                        // End of stream -- send empty frame with EOS flag set.
                        decoder.queueInputBuffer(inputBufIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        inputDone = true;
                    } else {
                        long presentationTimeUs = extractor.getSampleTime();
                        decoder.queueInputBuffer(inputBufIndex, 0, chunkSize, presentationTimeUs, 0);
                        inputDone = ! extractor.advance();
                    }
                }
            }

            if (!outputDone) {
                ByteBuffer output;
                int decoderStatus = decoder.dequeueOutputBuffer(info, TIMEOUT_USEC);
                if (decoderStatus < 0)
                    throw new RuntimeException("Unexpected result from decoder.dequeueOutputBuffer: " + decoderStatus);
                else {
                    if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                        output = decoder.getOutputBuffer(decoderStatus);
                    else {
                        output = outputBuffers[decoderStatus];
                        output.position(info.offset);
                    }
                }

                processOutputBuffer(decoderStatus, output, info);

                if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0)
                    outputDone = true;
            }
        }
    }


    protected abstract void processOutputBuffer(int decoderStatus, ByteBuffer buffer, MediaCodec.BufferInfo info);

}
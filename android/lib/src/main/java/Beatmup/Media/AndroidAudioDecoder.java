package Beatmup.Media;

import android.media.AudioTrack;
import android.media.MediaCodec;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by HomePlaneR on 15/05/2016.
 */
public class AndroidAudioDecoder extends AndroidMediaDecoder {
    public AndroidAudioDecoder(String filename) throws IOException {
        super(filename);
    }

    @Override
    protected void processOutputBuffer(int decoderStatus, ByteBuffer buffer, MediaCodec.BufferInfo info) {

    }


}

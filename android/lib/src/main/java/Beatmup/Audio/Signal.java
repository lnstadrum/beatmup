package Beatmup.Audio;

import Beatmup.Context;
import Beatmup.Sequence;

/**
 * A sound!
 */
public class Signal extends Sequence {
    private static native long newAudioSignal(Context context, int format, int sampleRate, int channels, float fragmentLen);
    private static native long newAudioSignalFromWAV(Context context, String filename);
    private static native long newAudioSignalSource(Context context, long signalHandle);

    public static class Source extends Beatmup.Audio.Source.AbstractSource {
        public Source(Context context, Signal signal) {
            super(newAudioSignalSource(context, signal.handle));
        }
    }

    protected long getHandle() {
        return handle;
    }

    public Signal(Context context, SampleFormat format, int sampleRate, int channelCount, float fragmentLen) {
        super(newAudioSignal(context, format.ordinal(), sampleRate, channelCount, fragmentLen));
    }

    public Signal(Context context, String WAVFilename) {
        super(newAudioSignalFromWAV(context, WAVFilename));
    }
}

package Beatmup.Audio;

import Beatmup.*;
import Beatmup.Audio.Source.AbstractSource;
import Beatmup.Exceptions.PlaybackException;

/**
 * Audio playback
 */
public class Playback extends Task {
    private static native long newPlayback(Context context);
    private native void configure(long handle, int sampleFormat, int sampleRate,
                                  int channelCount, int numBuffers, int bufferLength);
    private native void start(long handle);
    private native void stop(long handle);
    private native void setSource(long handle, AbstractSource source);

    private AbstractSource source;


    /**
     * Creates a new playback
     * @param context   a Beatmup engine context
     */
    public Playback(Context context) {
        super(context, newPlayback(context));
    }

    /**
     * Initializes playback setting specified parameters of audio output
     * @param sampleFormat      output sample format
     * @param sampleRate        output sample rate in Hz (samples per seconds)
     * @param numChannels       number of channels (e.g., 1 for mono, 2 for stereo)
     * @param numBuffers        number of atomic buffers, must be at least 2
     * @param bufferLength      length of each atomic buffer in samples
     * @throws PlaybackException if the parameters are incorrect or do not match hardware capabilities
     */
    public void configure(SampleFormat sampleFormat, int sampleRate, int numChannels, int numBuffers, int bufferLength)
            throws PlaybackException
    {
        configure(handle, sampleFormat.ordinal(), sampleRate, numChannels, numBuffers, bufferLength);
    }


    public void setSource(AbstractSource source) {
        this.source = source;
        setSource(handle, source);
    }


    public AbstractSource getSource() {
        return source;
    }


    public void start() throws PlaybackException {
        start(handle);
    }


    public void stop() throws PlaybackException {
        stop(handle);
    }
}

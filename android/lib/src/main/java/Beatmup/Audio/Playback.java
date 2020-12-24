/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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

package Beatmup.Audio;

import Beatmup.*;
import Beatmup.Exceptions.PlaybackException;

/**
 * Abstract audio playback base class.
 * A Task incarnation that samples a given AbstractSource in a buffered fashion.
 */
public class Playback extends Task {
    private static native long newPlayback(Context context);
    private native void initialize(long handle, int sampleRate, int sampleFormat, int channelCount, int numBuffers, int bufferLength) throws PlaybackException;
    private native void start(long handle) throws PlaybackException;
    private native void stop(long handle) throws PlaybackException;
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
     * Sets playback configuration.
     * @param sampleRate        sampling frequency in Hz
     * @param sampleFormat      sample format
     * @param numChannels       number of channels (e.g., 1 for mono, 2 for stereo)
     * @param bufferLength      length of each atomic buffer in samples
     * @param numBuffers        number of atomic buffers, at least 2
     * @throws PlaybackException if the parameters are incorrect or do not match hardware capabilities
     */
    public void initialize(int sampleRate, SampleFormat sampleFormat, int numChannels, int bufferLength, int numBuffers) throws PlaybackException
    {
        initialize(handle, sampleRate, sampleFormat.ordinal(), numChannels, bufferLength, numBuffers);
    }

    /**
     * Specifies a Source to sample.
     * @param source        The source of audio signal to play from
     */
    public void setSource(AbstractSource source) {
        this.source = source;
        setSource(handle, source);
    }

    /**
     * @return the signal source to sample.
     */
    public AbstractSource getSource() {
        return source;
    }

    /**
     * Starts playback.
     * @throws PlaybackException if something goes wrong with the output audio device used.
     */
    public void start() throws PlaybackException {
        start(handle);
    }

    /**
     * Starts playback.
     * @throws PlaybackException if something goes wrong with the output audio device used.
     */
    public void stop() throws PlaybackException {
        stop(handle);
    }
}

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

import java.io.IOError;

import Beatmup.Context;
import Beatmup.Sequence;

/**
 * An audio signal.
 */
public class Signal extends Sequence {
    private static native long newAudioSignal(Context context, int format, int sampleRate, int channels, float fragmentLen);
    private static native long newAudioSignalFromWAV(Context context, String filename) throws IOError;
    private static native long newAudioSignalSource(Context context, long signalHandle);
    private static native int getDuration(long handle);
    private static native int getSampleFormat(long handle);
    private static native int getChannelCount(long handle);

    protected long getHandle() {
        return handle;
    }

    /**
     * Creates an empty signal
     * @param context       A Context instance the new signal is associated to
     * @param format        Sample format of the new signal
     * @param sampleRate    Sample rate, Hz
     * @param channelCount  Number of channels
     * @param fragmentLen   Default fragment length in seconds
     */
    public Signal(Context context, SampleFormat format, int sampleRate, int channelCount, float fragmentLen) {
        super(newAudioSignal(context, format.ordinal(), sampleRate, channelCount, fragmentLen));
    }

    /**
     * Creates an AudioSignal reading a PCM-encoded WAV file.
     * @param context       A Context instance the new signal is associated to
     * @param wavFilename   Path to the WAV file to read
     * @throws java.io.IOError if cannot read the file.
     */
    public Signal(Context context, String wavFilename) throws IOError {
        super(newAudioSignalFromWAV(context, wavFilename));
    }

    /**
     * @return signal duration in number of samples.
     */
    public int getDuration() {
        return getDuration(handle);
    }

    /**
     * @return sample format of the signal.
     */
    public SampleFormat getSampleFormat() {
        return SampleFormat.values()[ getSampleFormat(handle) ];
    }

    /**
     * @return number of channels in the signal.
     */
    public int getChannelCount() {
        return getChannelCount(handle);
    }

    /**
     * AbstractSource reading samples from a given Signal.
     */
    public static class Source extends AbstractSource {
        /**
         * Creates a new Source serving a specific Signal.
         * @param context       A Beatmup context instance
         * @param signal        The signal
         */
        public Source(Context context, Signal signal) {
            super(newAudioSignalSource(context, signal.handle));
        }
    }
}

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

package Beatmup.Audio;

/**
 * An AbstractSource producing a sinusoidal signal, mainly for test purposes.
 */
public class HarmonicSource extends AbstractSource {
    private static native long newHarmonicSource();
    private native void setFrequency(long handle, float hz);
    private native void setPhase(long handle, float radians);
    private native void setAmplitude(long handle, float amp);

    /**
     * Creates a new harmonic source.
     */
    public HarmonicSource() {
        super(newHarmonicSource());
    }

    /**
     * Sets frequency of the harmonic produced by the signal source.
     * @param hz    The frequency in Hertz
     */
    public void setFrequency(float hz) {
        setFrequency(handle, hz);
    }

    /**
     * Sets the initial phase of the harmonic produced by the signal source.
     * @param rad   The phase in radians
     */
    public void setPhase(float rad) {
        setPhase(handle, rad);
    }

    /**
     * Sets the normalized amplitude of the harmonic produced by the signal source.
     * @param amp   The amplitude value. 1 or -1 corresponds to the maximum available dynamics without clipping.
     */
    public void setAmplitude(float amp) {
        setAmplitude(handle, amp);
    }
}

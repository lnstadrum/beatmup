package Beatmup.Audio.Source;

/**
 * A sinusoidal signal generator, mainly for test purposes
 */
public class Harmonic extends Source {
    private static native long newHarmonicSource();
    private native void setFrequency(long handle, float hz);
    private native void setPhase(long handle, float radians);
    private native void setAmplitude(long handle, float amp);

    public Harmonic() {
        super(newHarmonicSource());
    }

    public void setFrequency(float hz) {
        setFrequency(handle, hz);
    }

    public void setPhase(float rad) {
        setPhase(handle, rad);
    }

    public void setAmplitude(float amp) {
        setAmplitude(handle, amp);
    }
}

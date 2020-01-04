package Beatmup.Utils;

/**
 * Limits speed
 */
public class SpeedLimiter {
    private boolean set;
    private long lastTick, lastMeasurement;

    public SpeedLimiter() {
        reset();
    }

    /**
     * Resets the speed limiter
     */
    public void reset() {
        set = false;
    }

    /**
     * Blocks until a specific time is reached.
     * The very first call (after reset()) is non-blocking.
     * @param whenUs the time marker in microseconds to wait till.
     */
    public void tick(long whenUs) {
        final long TOLERANCE_US = 1000L;
        if (set) {
            long dt = (whenUs - lastTick) - (System.nanoTime() / 1000L - lastMeasurement) - TOLERANCE_US;
            try {
                if (dt > 0)
                    Thread.sleep(dt / 1000L, 1000 * (int)(dt % 1000L));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        else
            set = true;
        lastMeasurement = System.nanoTime() / 1000L;
        lastTick = whenUs;
    }
}

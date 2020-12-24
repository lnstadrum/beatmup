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

package Beatmup.Utils;

/**
 * Limits caller speed by capping invocation frequency.
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

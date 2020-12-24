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

package Beatmup.Android;

/**
 * Beatmup engine context for Android
 */
public class Context extends Beatmup.Context {
    private static native long newContext(int numPools);

    /**
     * Creates a new Context.
     * @param poolCount     Number of thread pools to run tasks. Equals to the max number of tasks running in parallel.
     */
    public Context(final int poolCount) {
        super(newContext(poolCount));
    }


    @Override
    public synchronized void dispose() {
        super.dispose();
    }
}

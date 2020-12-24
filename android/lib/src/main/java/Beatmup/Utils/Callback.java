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

import Beatmup.Context;
import Beatmup.Task;

/**
 * Task calling a function back from the core.
 * Convenient as a part of {@link Beatmup.Pipelining.Multitask}.
 */
public abstract class Callback extends Task {
    private static native long newCallbackTask();
    private native void updateCallback();

    /**
     * Creates a callback task instance
     * @param context       A Beatmup context instance
     */
    public Callback(Context context) {
        super(context, newCallbackTask());
        updateCallback();
    }

    /**
     * Method called during the task execution
     */
    public abstract void run();
}

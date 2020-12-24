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

package Beatmup;

import Beatmup.Exceptions.CoreException;

/**
 * Abstract task
 */
public class Task extends Object {
    protected Context context;

    protected Task(Context context, long handle) {
        super(handle);
        this.context = context;
    }

    /**
     * @return a Beatmup Context instance the task is associated with.
     */
    public Context getContext() {
        return context;
    }

    /**
     * Runs the task
     * @return execution time in ms
     */
    public float execute() throws CoreException {
        return context.performTask(this);
    }
}

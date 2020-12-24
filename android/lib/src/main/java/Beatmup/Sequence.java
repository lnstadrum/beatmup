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

/**
 * An editable sequence defined in time
 */
public class Sequence extends Beatmup.Object {

    private native long copy(long handle, int start, int end);
    private native void insert(long handle, long anotherHandle, int time);
    private native void remove(long handle, int start, int end);
    private native void shrink(long handle, int start, int end);

    protected Sequence(long handle) {
        super(handle);
    }

    /**
     * Extracts a subsequence by creating a copy
     * @param start     the copied part start time
     * @param end       the copied part end time
     * @return new sequence containing the copy
     */
    public Sequence copy(int start, int end) {
        return new Sequence(copy(handle, start, end));
    }

    /**
     * Inserts another sequence in the current one
     * @param sequence      the new data to insert
     * @param time          time moment where the new sequence will be inserted
     */
    public void insert(Sequence sequence, int time) {
        insert(handle, sequence.handle, time);
    }

    /**
     * Removes out a part from the sequence
     * @param start     the start time moment to remove data from
     * @param end       the end time moment
     */
    public void remove(int start, int end) {
        remove(handle, start, end);
    }
}

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

import java.io.IOError;

import Beatmup.Object;

/**
 * A key-value pair set storing pieces of arbitrary data (chunks) under string keys.
 * A chunk is a header and a piece of data packed in memory like this: (idLength[4], id[idLength], size[sizeof(chunksize_t)], data[size])
 * ChunkCollection defines an interface to retrieve chunks by their ids.
 */
public class ChunkCollection extends Object {
    private native void open(long handle) throws IOError;
    private native void close(long handle);
    private native long size(long handle);
    private native boolean chunkExists(long handle, String id);
    private native long chunkSize(long handle, String id);
    private native void save(long handle, String filename, boolean append);
    private native String read(long handle, String id);

    protected ChunkCollection(long handle) {
        super(handle);
    }

    /**
     * Opens the collection to read chunks from it.
     */
    public void open() throws IOError {
        open(handle);
    }

    /**
     * Closes the collection after a reading session.
     */
    public void close() {
        close(handle);
    }

    /**
     * @return number of chunks in the collection.
     */
    public long size() {
        return size(handle);
    }

    /**
     * Check if a specific chunk exists.
     * @param id        The chunk id
     * @return `true` if the chunk exists in the collection.
     */
    public boolean chunkExists(String id) {
        return chunkExists(handle, id);
    }

    /**
     * Retrieves size of a specific chunk.
     * @param id        The chunk id
     * @return size of the chunk in bytes, 0 if not found.
     */
    public long chunkSize(String id) {
        return chunkSize(handle, id);
    }

    /**
     * Reads a chunk with a specific id and returns it as a string.
     * If there is no chunk with the given id, returns an empty string.
     * @param id        The chunk id
     * @return the chunk content as string.
     */
    public String read(String id) {
        return read(handle, id);
    }
}

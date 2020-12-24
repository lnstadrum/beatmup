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

/**
 * File containing chunks.
 * The file is not loaded in memory, but is scanned when first opened to collect the information about available chunks.
 */
public class ChunkFile extends ChunkCollection {
    private static native long newChunkfile(String filename, boolean openNow) throws IOError;

    /**
     * Creates a read-only chunk collection from a file.
     * @param filename      The file name / path
     * @param openNow       If `true`, the file is read right away. Otherwise it is done on open() call.
     *                      No information is available about chunks in the file until it is opened.
     */
    public ChunkFile(String filename, boolean openNow) throws IOError {
        super(newChunkfile(filename, openNow));
    }
}

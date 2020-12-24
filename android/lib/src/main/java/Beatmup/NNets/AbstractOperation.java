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

package Beatmup.NNets;

/**
 * Abstract neural net operation (layer).
 * Has a name used to refer the operation in a Model. The operation data (such as convolution weights) is provided through a ChunkCollection
 * in single precision floating point format, where the chunks are searched by operation name.
 * Operations have several inputs and outputs numbered starting from zero.
 */
public class AbstractOperation {
    private native String getName(long handle);
    private native long countMultiplyAdds(long handle);
    protected static native long getOperationFromModel(long modelHandle, String name) throws IllegalArgumentException;

    long handle;    //!< pointer to the native op; AbstractOperation instances are fully managed in the native code; this class only wraps an existing native object

    protected AbstractOperation(long handle) {
        this.handle = handle;
    }

    /**
     * @return operation name.
     */
    public String getName() {
        return getName(handle);
    }

    /**
     * @return (approximate) number of multiplications and additions used by this operation.
     */
    public long countMultiplyAdds() {
        return countMultiplyAdds(handle);
    }
}

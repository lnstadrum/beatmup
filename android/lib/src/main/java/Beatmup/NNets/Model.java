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

import Beatmup.Object;

/**
 * Neural net model.
 * Contains a list of operations and programmatically defined interconnections between them using addConnection().
 * Enables access to the model memory at any point in the model through addOutput() and getModelData().
 * The memory needed to store internal data during the inference is allocated automatically; storages are reused when possible.
 * The inference of a Model is performed by InferenceTask.
 */
public class Model extends Object {
    // native methods
    private native String serializeToString(long handle);
    private native long countMultiplyAdds(long handle);
    private native long getMemorySize(long handle);
    private native long getNumberOfOperations(long handle);

    long getHandle() {
        return handle;
    }

    protected Model(long handle) {
        super(handle);
    }

    /**
     * @return serialized representation of the model as a Listing.
     */
    public String serializeToString() {
        return serializeToString(handle);
    }

    /**
     * Provides an estimation of the number of multiplication and addition operations characterizing the model complexity.
     * Queries the number of multiply-adds of every operation of the model and sums them up.
     * @return (approximate) number of multiplications and additions used in the model.
     */
    public long countMultiplyAdds() {
        return countMultiplyAdds(handle);
    }

    /**
     * @return the amount of texture memory in bytes currently allocated by the model to run the inference.
     * When the model is ready to run, this represents the size of the memory needed to store internal data during the inference.
     * The resulting value does not include the size of GLSL shaders binaries stored in GPU memory which can be significant.
     */
    public long getMemorySize() {
        return getMemorySize(handle);
    }

    /**
     * @return number of operations in the model.
     */
    public long getNumberOfOperations() {
        return getNumberOfOperations(handle);
    }
}

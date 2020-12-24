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
 * Softmax operation
 */
public class Softmax extends AbstractOperation {
    private static native float[] getProbabilities(long handle);

    private Softmax(long handle) {
        super(handle);
    }

    /**
     * Retrieves a Softmax operation in the model.
     * This search operation is not type-safe. If the original operation in the model is not a Softmax, using the returned instance may lead to a memory corruption or illegal
     * memory access.
     * @param model         The model
     * @param name          The operation name
     * @return Softmax operation instance.
     * @throws IllegalArgumentException if no operation with the given name found in the model.
     */
    public static Softmax fromModel(Model model, String name) throws IllegalArgumentException {
        return new Softmax(getOperationFromModel(model.getHandle(), name));
    }


    /**
     * @return array of probability values on Softmax output.
     */
    public float[] getProbabilities() {
        return getProbabilities(handle);
    }
}
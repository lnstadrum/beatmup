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

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;
import Beatmup.Utils.ChunkCollection;

/**
 * Task running inference of a Model
 */
public class InferenceTask extends Task {
    private static native long newInferenceTask(Model model, ChunkCollection modelData);
    private native void connectByName(long handle, Bitmap image, String operation, int inputIndex) throws IllegalArgumentException;
    private native void connectByHandle(long handle, Bitmap image, long operation, int inputIndex);

    /**
     * Creates an instance of InferenceTask.
     * @param context       A Beatmup context instance
     * @param model         The model to run inference of
     * @param modelData     The model data
     */
    public InferenceTask(Context context, Model model, ChunkCollection modelData) {
        super(context, newInferenceTask(model, modelData));
    }

    /**
     * Connects an image to a specific operation input.
     * Ensures the image content is up-to-date in GPU memory by the time the inference is run.
     * @param image         The image
     * @param operation     The operation name
     * @param inputIndex    The input index of the operation
     * @throws IllegalArgumentException if no operation with the given name is found in the model.
     */
    public void connect(Bitmap image, String operation, int inputIndex) throws IllegalArgumentException {
        connectByName(handle, image, operation, inputIndex);
    }

    /**
     * Connects an image to a specific operation input.
     * Ensures the image content is up-to-date in GPU memory by the time the inference is run.
     * @param image         The image
     * @param operation     The operation
     * @param inputIndex    The input index of the operation
     */
    public void connect(Bitmap image, AbstractOperation operation, int inputIndex) {
        connectByHandle(handle, image, operation.handle, inputIndex);
    }
}

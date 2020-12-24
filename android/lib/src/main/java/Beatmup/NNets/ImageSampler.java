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
 * Image preprocessing operation.
 * Samples an image of a fixed size from an arbitrary size texture. Has three key missions.
 *  - If enabled, performs a center crop keeping the output aspect ratio (otherwise the input is stretched to fit the output).
 *  - If enabled, uses linear interpolation when possible to reduce aliasing (otherwise nearest neighbor sampling is used).
 *  - Brings support of OES textures. This allows for example to read data directly from camera in Android.
 */
public class ImageSampler extends AbstractOperation {
    private static native void setRotation(long handle, int rotation);
    private static native int getRotation(long handle);

    private ImageSampler(long handle) {
        super(handle);
    }

    /**
     * Retrieves a ImageSampler operation in the model.
     * This search operation is not type-safe. If the original operation in the model is not a ImageSampler, using the returned instance may lead to a memory corruption or illegal
     * memory access.
     * @param model         The model
     * @param name          The operation name
     * @return ImageSampler operation instance.
     * @throws IllegalArgumentException if no operation with the given name found in the model.
     */
    public static ImageSampler fromModel(Model model, String name) throws IllegalArgumentException {
        return new ImageSampler(getOperationFromModel(model.getHandle(), name));
    }

    /**
     * Specifies a rotation to apply to the input image.
     * @param rotation      Number of times a clockwise rotation by 90 degree is applied to the input image.
     */
    public void setRotation(int rotation) {
        setRotation(handle, rotation);
    }

    /**
     * Returns rotation applied to the input image.
     * @return number of times a clockwise rotation by 90 degree is applied to the input image.
     */
    public int getRotation() {
        return getRotation(handle);
    }
}


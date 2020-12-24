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

package Beatmup.Imaging;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Geometry.IntPoint;
import Beatmup.Task;

/**
 * A binary operation on images.
 * Evaluates expression O = op(L, R) for bitmaps L, R, O and a given pixelwise operation op.
 * Allows to select the operating area in all the three bitmaps.
 */
public class BinaryOperation extends Task {
    private static native long newBinaryOperation();
    private native void setOperand1(long handle, Bitmap op1);
    private native void setOperand2(long handle, Bitmap op2);
    private native void setOutput(long handle, Bitmap output);
    private native void setOperation(long handle, int operation);

    private native void resetCrop(long handle);
    private native void setCrop(long handle,
            int width, int height,
            int op1X, int op1Y,
            int op2X, int op2Y,
            int outX, int outY);
    private native void getCrop(long handle, IntPoint size, IntPoint op1Origin, IntPoint op2Origin, IntPoint outOrigin);

    private Bitmap leftOperand, rightOperand, output;
    private Operation operation;

    /**
     * Binary operation specification.
     */
    public enum Operation {
        NONE,       //!< bypass; the output bitmap remains unchanged
        ADD,        //!< the input images are added
        MULTIPLY    //!< the input images are multiplied
    }

    /**
     * Intantiates a binary image operation.
     * @param context       A Beatmup context instance
     */
    public BinaryOperation(Context context) {
        super(context, newBinaryOperation());
    }

    /**
     * Sets left operand bitmap (L).
     * @param bitmap    The bitmap
     */
    public void setLeftOperand(Bitmap bitmap) {
        this.leftOperand = bitmap;
        setOperand1(handle, bitmap);
    }

    /**
     * Sets right operand bitmap (R).
     * @param bitmap    The bitmap
     */
    public void setRightOperand(Bitmap bitmap) {
        this.rightOperand = bitmap;
        setOperand2(handle, bitmap);
    }

    /**
     * Sets a bitmap to store the operation result.
     * @param bitmap    The bitmap
     */
    public void setOutput(Bitmap bitmap) {
        this.output = bitmap;
        setOutput(handle, bitmap);
    }

    /**
     * @return left operand or null if not set.
     */
    public Bitmap getLeftOperand() {
        return leftOperand;
    }

    /**
     * @return right operand or null if not set.
     */
    public Bitmap getRightOperand() {
        return rightOperand;
    }

    /**
     * @return output bitmap or null if not set.
     */
    public Bitmap getOutput() {
        return output;
    }

    /**
     * Sets the operation to perform on input images.
     * @param operation     The operation
     */
    public void setOperation(Operation operation) {
        this.operation = operation;
        setOperation(handle, operation.ordinal());
    }

    /**
     * @return the currently selected operation.
     */
    public Operation getOperation() {
        return operation;
    }

    /**
     * Specifies a rectangular operating area.
     * Only pixels within a specific rectangle in left and right operands are used. The result is put in a rectangular area of the same size in the output bitmap (the remaining
     * pixels are unchanged).
     * @param width             Width in pixels
     * @param height            Height in pixels.
     * @param leftOrigin        Position of the operating area in the left operand.
     * @param rightOrigin       Position of the operating area in the right operand.
     * @param outputOrigin      Position of the operating area in the output image.
     */
    public void setCrop(int width, int height, IntPoint leftOrigin, IntPoint rightOrigin, IntPoint outputOrigin) {
        setCrop(handle,
                width, height,
                leftOrigin.x, leftOrigin.y,
                rightOrigin.x, rightOrigin.y,
                outputOrigin.x, outputOrigin.y);
    }

    /**
     * Resets the operating area position and size to use the full surface of the input images.
     * If the input images are not of the same size as the output, an exception is thrown during the task execution.
     */
    public void resetCrop() {
        resetCrop(handle);
    }
}

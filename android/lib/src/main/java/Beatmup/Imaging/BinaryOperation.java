package Beatmup.Imaging;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Geometry.IntPoint;
import Beatmup.Task;

public class BinaryOperation extends Task {
    private static native long newBinaryOperation();
    private native void setOperand1(long handle, Bitmap op1);
    private native void setOperand2(long handle, Bitmap op2);
    private native void setOutput(long handle, Bitmap output);
    private native void setOperation(long handle, int operation);

    private native void resetCrop(long handle);
    private native void setCrop(
            long handle,
            int width, int height,
            int op1X, int op1Y,
            int op2X, int op2Y,
            int outX, int outY);
    private native void getCrop(
            long handle,
            IntPoint size, IntPoint op1Origin, IntPoint op2Origin, IntPoint outOrigin);

    private Bitmap leftOperand, rightOperand, output;
    private Operation operation;

    public static enum Operation {
        NONE,
        ADD,
        MULTIPLY
    }

    public BinaryOperation(Context context) {
        super(context, newBinaryOperation());
    }

    public void setLeftOperand(Bitmap bitmap) {
        this.leftOperand = bitmap;
        setOperand1(handle, bitmap);
    }

    public void setRightOperand(Bitmap bitmap) {
        this.rightOperand = bitmap;
        setOperand2(handle, bitmap);
    }

    public void setOutput(Bitmap bitmap) {
        this.output = bitmap;
        setOutput(handle, bitmap);
    }

    public Bitmap getLeftOperand() {
        return leftOperand;
    }

    public Bitmap getRightOperand() {
        return rightOperand;
    }

    public Bitmap getOutput() {
        return output;
    }

    public void setOperation(Operation operation) {
        this.operation = operation;
        setOperation(handle, operation.ordinal());
    }

    public Operation getOperation() {
        return operation;
    }

    public void resetCrop() {
        resetCrop(handle);
    }

    public void setCrop(int width, int height, IntPoint leftOrigin, IntPoint rightOrigin, IntPoint outputOrigin) {
        setCrop(handle,
                width, height,
                leftOrigin.x, leftOrigin.y,
                rightOrigin.x, rightOrigin.y,
                outputOrigin.x, outputOrigin.y);
    }
}

package Beatmup.Imaging;

import java.util.List;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Exceptions.NullPointer;
import Beatmup.Geometry.IntPoint;
import Beatmup.Geometry.IntRectangle;
import Beatmup.Geometry.Rectangle;
import Beatmup.Task;

/**
 * Flood filling
 */
public class FloodFill extends Task {
    public enum BorderMorphology {
        NONE,
        DILATE,
        ERODE
    }

    // references to avoid GC
    private Bitmap input, output;

    private static native long newFloodFill();

    private native void setInput(long handle, Bitmap bitmap);

    private native void setOutput(long handle, Bitmap bitmap);

    private native void setMaskPos(long handle, int x, int y);

    private native void setTolerance(long handle, float tolerance);

    private native void setBorderPostprocessing(long handle, int operation, float holdRadius, float releaseRadius);

    private native void setSeeds(long handle, int[] xy);

    private native void setComputeContours(long handle, boolean enable);

    private native int getContourCount(long handle);

    private native int getContourPointCount(long handle, int seed);

    private native int[] getContour(long handle, int seed, float step);

    private native IntRectangle getBoundingBox(long handle);

    /**
     * Creates new region mask renderer
     * @param context       Beatmup core context
     */
    public FloodFill(Context context) {
        super(context, newFloodFill());
    }

    public void setInput(Bitmap bitmap) {
        this.input = bitmap;
        setInput(handle, bitmap);
    }

    public void setOutput(Bitmap bitmap) {
        this.output = bitmap;
        setOutput(handle, bitmap);
    }

    public void setMaskPos(int x, int y) {
        setMaskPos(handle, x, y);
    }

    public void setTolerance(float tolerance) {
        setTolerance(handle, tolerance);
    }

    public void setBorderPostprocessing(BorderMorphology operation, float holdRadius, float releaseRadius) {
        if (holdRadius < 0)
            throw new IllegalArgumentException("Hold radius must not be negative.");
        if (releaseRadius < holdRadius)
            throw new IllegalArgumentException("Release radius must be greater than hold radius.");
        setBorderPostprocessing(handle, operation.ordinal(), holdRadius, releaseRadius);
    }

    public void setSeeds(List<IntPoint> seeds) {
        if (seeds.size() == 0)
            throw new IllegalArgumentException("Seed set is empty.");
        int[] xy = new int[seeds.size()*2];
        for (int i = 0; i < seeds.size(); i++) {
            xy[2*i  ] = seeds.get(i).x;
            xy[2*i+1] = seeds.get(i).y;
        }
        setSeeds(handle, xy);
    }

    public void setComputeContours(boolean enable) {
        setComputeContours(handle, enable);
    }

    public Bitmap getInput() {
        return input;
    }

    public Bitmap getOutput() {
        return output;
    }

    public int getContourCount() { return getContourCount(handle); }

    public int getContourPointCount(int seedIndex) {
        return getContourPointCount(handle, seedIndex);
    }

    /**
     * Returns border of a connected component by its seed
     * @param seedIndex         the seed index
     * @return pairs of point coordinates (x,y) forming the border contour
     */
    public int[] getContour(int seedIndex) {
        return getContour(handle, seedIndex, 0);
    }

    /**
     * Returns border of a connected component by its seed
     * @param seedIndex         the seed index
     * @param step              min interpoint distance, serves to skip some points
     * @return pairs of point coordinates (x,y) forming the border contour
     */
    public int[] getContour(int seedIndex, float step) {
        return getContour(handle, seedIndex, step);
    }

    /**
     * Crops zero parts of the output mask
     * @param normalizedCropRect        crop rectangle in normalized coordinates (if not null, will be set)
     * @return a new bitmap containing cropped mask
     */
    public Bitmap optimizeMask(Rectangle normalizedCropRect) {
        if (output == null)
            throw new NullPointer("Output bitmap is not set.");
        IntRectangle boundingBox = getBoundingBox(handle);
        boundingBox.x2++;
        boundingBox.y2++;
        Bitmap result = output.copyRegion(boundingBox);
        if (normalizedCropRect != null) {
            normalizedCropRect.x1 = (float) boundingBox.x1 / output.getWidth();
            normalizedCropRect.y1 = (float) boundingBox.y1 / output.getWidth();
            normalizedCropRect.x2 = (float) (boundingBox.x1 + result.getWidth()) / output.getWidth();
            normalizedCropRect.y2 = (float) (boundingBox.y1 + result.getHeight()) / output.getWidth();
        }
        return result;
    }
}

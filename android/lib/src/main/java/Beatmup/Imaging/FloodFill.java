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

import java.util.List;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Exceptions.CoreException;
import Beatmup.Exceptions.NullPointer;
import Beatmup.Geometry.IntPoint;
import Beatmup.Geometry.IntRectangle;
import Beatmup.Geometry.Rectangle;
import Beatmup.Task;

/**
 Flood fill algorithm implementation.
 Discovers areas of similar colors up to a tolerance threshold around given positions (seeds) in the input image. These areas are filled with white color in another image (output).
 If the output bitmap is a binary mask, corresponding pixels are set to `1`. The rest of the output image remains unchanged.
 Optionally, computes contours around the discovered areas and stores the contour positions. Also optionally, applies post-processing by dilating or eroding the discovered regions
 in the output image.
 */
public class FloodFill extends Task {
    /**
     * Morphological postprocessing operation applied to discovered connected components
     */
    public enum BorderMorphology {
        NONE,
        DILATE,
        ERODE
    }

    // references to protect from GC
    private Bitmap input, output;

    // native methods
    private static native long newFloodFill();
    private native void setInput(long handle, Bitmap bitmap);
    private native void setOutput(long handle, Bitmap bitmap);
    private native void setMaskPos(long handle, int x, int y);
    private native void setTolerance(long handle, float tolerance);
    private native void setBorderPostprocessing(long handle, int operation, float holdRadius, float releaseRadius);
    private native void setSeeds(long handle, int[] xy);
    private native void setComputeContours(long handle, boolean enable);
    private native int getContourCount(long handle);
    private native int getContourPointCount(long handle, int seed) throws CoreException;
    private native int[] getContour(long handle, int seed, float step) throws CoreException;
    private native IntRectangle getBoundingBox(long handle);

    /**
     * Creates new instance of FloodFill
     * @param context       A Context instance
     */
    public FloodFill(Context context) {
        super(context, newFloodFill());
    }

    /**
     * Sets the input image.
     * The input image is read to discover connected components of similar colors, but remains unchanged.
     * @param bitmap    The image
     */
    public void setInput(Bitmap bitmap) {
        this.input = bitmap;
        setInput(handle, bitmap);
    }

    /**
     * Sets the output image.
     * The output image contains masks of discovered connected components from a given set of seed: the corresponding pixel values are set to the maximum value of the output image
     * pixel format. The rest of its pixels remains unchanged.
     * @param bitmap    The image
     */
    public void setOutput(Bitmap bitmap) {
        this.output = bitmap;
        setOutput(handle, bitmap);
    }

    /**
     * Specifies left-top corner position of the mask to compute inside the input bitmap.
     * @param x     Horizontal coordinate (offset)
     * @param y     Vertical coordinate (offset)
     */
    public void setMaskPos(int x, int y) {
        setMaskPos(handle, x, y);
    }

    /**
     * Sets the intensity tolerance threshold used to decide on similarity of neighboring pixels.
     * @param tolerance     The threshold
     */
    public void setTolerance(float tolerance) {
        setTolerance(handle, tolerance);
    }

    /**
     * Specifies a morphological operation to apply to the mask border.
     * @param operation         A postprocessing operation
     * @param holdRadius        Erosion/dilation hold radius (output values set to 1)
     * @param releaseRadius     Erosion/dilation radius of transition from 1 to 0
     */
    public void setBorderPostprocessing(BorderMorphology operation, float holdRadius, float releaseRadius) {
        if (holdRadius < 0)
            throw new IllegalArgumentException("Hold radius must not be negative.");
        if (releaseRadius < holdRadius)
            throw new IllegalArgumentException("Release radius must be greater than hold radius.");
        setBorderPostprocessing(handle, operation.ordinal(), holdRadius, releaseRadius);
    }

    /**
     * Communicates a set of seed points.
     * The research of connected components of similar colors starts in specific locations in the input image (seeds)
     * @param seeds     The list of seeds
     */
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

    /**
     * Enables or disables contours computation.
     * @param enable    If `true`, the contours of discovered connected components are computed.
     */
    public void setComputeContours(boolean enable) {
        setComputeContours(handle, enable);
    }

    /**
     * @return the input image.
     */
    public Bitmap getInput() {
        return input;
    }

    /**
     * @return the output image.
     */
    public Bitmap getOutput() {
        return output;
    }

    /**
     * @return number of detected contours. If the contour detection was not enabled with setComputeContours() before running the task, returns zero.
     */
    public int getContourCount() { return getContourCount(handle); }

    /**
     * Retrives a length of a specific contour.
     * @param index         Zero-based index of the contour. If out of range, an exception is thrown.
     * @return number of points in the contour.
     */
    public int getContourPointCount(int index) throws CoreException {
        return getContourPointCount(handle, index);
    }

    /**
     * Retrieves border contour of a connected component.
     * @param index          Zero-based index of the contour. If out of range, an exception is thrown.
     * @return pairs of point coordinates (x,y) forming the border contour
     */
    public int[] getContour(int index) throws CoreException {
        return getContour(handle, index, 0);
    }

    /**
     * Retrieves border contour of a connected component.
     * @param index         Zero-based index of the contour. If out of range, an exception is thrown.
     * @param step          Minimum distance between consecutive contour points, used to provide an approximated contour with reduced number of points
     * @return pairs of point coordinates (x,y) forming the border contour
     */
    public int[] getContour(int index, float step) throws CoreException {
        return getContour(handle, index, step);
    }

    /**
     * Crops zero parts of the output mask.
     * @param normalizedCropRect        The crop rectangle in normalized coordinates (if not null, will be set)
     * @return a new bitmap containing the cropped mask
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

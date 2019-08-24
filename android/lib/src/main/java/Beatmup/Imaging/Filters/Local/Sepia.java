package Beatmup.Imaging.Filters.Local;

import Beatmup.Context;
import Beatmup.Task;

/**
 * Simple sepia effect
 */
public class Sepia extends PixelwiseFilter {
    private static native long newSepia();

    public Sepia(Context context) {
        super(context, newSepia());
    }
}

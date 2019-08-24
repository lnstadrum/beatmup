package Beatmup.NNets;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

public class GPUBenchmark extends Task {
    private static native long newGpuBenchmark(Context context);
    private static native float getScore(long handle);
    private static native float getError(long handle);

    private static native float test(Context context, Bitmap test);

    public GPUBenchmark(Context context) {
        super(context, newGpuBenchmark(context));
    }


    public static void test(Bitmap bitmap, Context context) {
        test(context, bitmap);
    }

    public float getError() {
        return getError(handle);
    }

    public float getScore() {
        return getScore(handle);
    }
}

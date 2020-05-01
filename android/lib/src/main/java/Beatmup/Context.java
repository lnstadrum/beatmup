package Beatmup;

import java.io.File;
import java.util.HashMap;

import Beatmup.Geometry.IntPoint;
import Beatmup.Geometry.IntRectangle;
import Beatmup.Imaging.Color;
import Beatmup.Imaging.FloatColor;
import Beatmup.Imaging.PixelFormat;


/**
 * Beatmup engine context
 * Handles necessary data to interact with the engine through JNI layer
 */
public class Context extends Object {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("beatmup");
    }

    private final HashMap<Long, Bitmap> bitmaps;

    private long eventListenerHandle;

    // native methods
    private static native long attachEventListener(long ctx);
    private static native void detachEventListener(long eventListenerHandle);
    private static native long getTotalRam();

    private native float performTask(long ctx, int poolIndex, Task task);
    private native int submitTask(long ctx, int poolIndex, Task task);
    private native int submitPersistentTask(long ctx, int poolIndex, Task task);
    private native void repeatTask(long ctx, int poolIndex, Task task, boolean abortCurrent);
    private native void waitForJob(long ctx, int poolIndex, int job);
    private native boolean abortJob(long ctx, int poolIndex, int job);
    private native void waitForAllJobs(long ctx, int poolIndex);
    private native boolean busy(long ctx, int poolIndex);

    private native long renderChessboard(long ctx, int width, int height, int cellSize, int pixelFormat);
    private native long copyBitmap(Bitmap bitmap, int newPixelFormat);
    private native IntPoint scanlineSearchInt(long bitmap, int startX, int startY, int r, int g, int b, int a);
    private native IntPoint scanlineSearchFloat(long bitmap, int startX, int startY, float r, float g, float b, float a);

    private native int maxAllowedWorkerCount(long ctx, int poolIndex);
    private native void limitWorkerCount(long ctx, int poolIndex, int count);

    private native long swapOnDisk(long ctx, long howMuch);

    private native boolean isGPUQueried(long ctx);
    private native boolean isGPUReady(long ctx);
    private native void recycleGPUGarbage(long ctx);


    /**
     * Performs given task in the main thread pool.
     * @param task        the task
     * @return execution time in ms
     */
    public float performTask(Task task) {
        return performTask(handle, 0, task);
    }


    /**
     * Performs given task.
     * @param task          the task
     * @param poolIndex     index of the thread pool to run the task in
     * @return execution time in ms
     */
    public float performTask(Task task, int poolIndex) {
        return performTask(handle, poolIndex, task);
    }


    /**
     * Starts a  task or demands its repetition in the main thread pool.
     * @param task          the task
     * @param abortCurrent  if `true` and a task is running, abort signal is sent before repetition
     */
    public void repeatTask(Task task, boolean abortCurrent) {
        repeatTask(handle, 0, task, abortCurrent);
    }


    /**
     * Starts a  task or demands its repetition in a thread pool.
     * @param task          The task
     * @param abortCurrent  If `true` and a task is running, abort signal is sent before repetition
     * @param poolIndex     Index of the thread pool to run the task in
     */
    public void repeatTask(Task task, boolean abortCurrent, int poolIndex) {
        repeatTask(handle, poolIndex, task, abortCurrent);
    }


    /**
     * Submits a persistent task in the main thread pool.
     * @param task          The task
     * @return job index
     */
    public int submitPersistentTask(Task task) {
        return submitPersistentTask(handle, 0, task);
    }


    /**
     * Submits a persistent task in a specified thread pool.
     * @param task          The task
     * @param poolIndex     Thread pool index
     * @return job index
     */
    public int submitPersistentTask(Task task, int poolIndex) {
        return submitPersistentTask(handle, poolIndex, task);
    }


    /**
     * Blocks until a given job in the main thread pool finishes.
     * @param job           The job
     */
    public void waitForJob(int job) {
        waitForJob(handle, 0, job);
    }


    /**
     * Blocks until a given job in the main thread pool finishes.
     * @param job           The job
     * @param poolIndex     Thread pool index
     */
    public void waitForJob(boolean abort, int job, int poolIndex) {
        waitForJob(handle, poolIndex, job);
    }


    /**
     * Aborts a given submitted job.
     * @param job           The job
     * @return `true` if the job was interrupted while running.
     */
    public boolean abortJob(int job) {
        return abortJob(handle, 0, job);
    }


    /**
     * Aborts a given submitted job.
     * @param job           The job
     * @param poolIndex     Thread pool index
     * @return `true` if the job was interrupted while running.
     */
    public boolean abortJob(int job, int poolIndex) {
        return abortJob(handle, poolIndex, job);
    }


    /**
     * Sets maximum number of threads executing tasks in the main thread pool.
     * @param newCount      the new thread count limit
     */
    public void limitWorkerCount(int newCount) {
        limitWorkerCount(handle, 0, newCount);
    }


    /**
     * Sets maximum number of threads executing tasks in a specified thread pool.
     * @param newCount      the new thread count limit
     * @param poolIndex     index of the thread pool
     */
    public void limitWorkerCount(int newCount, int poolIndex) {
        limitWorkerCount(handle, poolIndex, newCount);
    }


    /**
     * Context initialization
     */
    protected Context(long handle) {
        super(handle);
        eventListenerHandle = attachEventListener(handle);
        bitmaps = new HashMap<>();
    }


    /**
     * Adds a bitmap to the watch list
     * @param bitmap    the new bitmap
     */
    protected void watchBitmap(Bitmap bitmap) {
        synchronized (bitmaps) {
            bitmaps.put(bitmap.handle, bitmap);
        }
    }


    /**
     * Removes a bitmap from the watch list
     * @param bitmap    the bitmap to remove
     */
    protected synchronized void unwatchBitmap(Bitmap bitmap) {
        synchronized (bitmaps) {
            bitmaps.remove(bitmap.handle);
        }
    }


    @Override
    public synchronized void dispose() {
        synchronized (bitmaps) {
            for (Bitmap bitmap : bitmaps.values()) {
                bitmap.dispose();
            }
        }
        recycleGPUGarbage();
        super.dispose();
        detachEventListener(eventListenerHandle);
    }


    /**
     * Renders a chessboard-like image
     * @param width         output image width in pixels
     * @param height        output image height in pixels
     * @param cellSize      chessboard cell size size in pixels
     * @param pixelFormat   output image pixel format
     * @return image of chessboard with cells aligned with topleft corner
     */
    public Bitmap renderChessboard(int width, int height, int cellSize, PixelFormat pixelFormat) {
        return new Bitmap(
                this,
                renderChessboard(handle, width, height, cellSize, pixelFormat.ordinal())
        );
    }


    /**
     * Creates a copy of given bitmap
     * @param source            the bitmap
     * @param pixelFormat       pixel format of the copy
     * @return copy of the bitmap with specified pixel format
     */
    public Bitmap copyBitmap(Bitmap source, PixelFormat pixelFormat) {
        return new Bitmap(this, copyBitmap(source, pixelFormat.ordinal()));
    }


    /**
     * Requests to swap some allocated memory on disk
     * @param howMuch   number of bytes to swap
     * @return actual number of swapped bytes
     */
    public long swapOnDisk(long howMuch) {
        return swapOnDisk(handle, howMuch);
    }


    /**
     * Runs through a bitmap in the scanline order (left to right, top to bottom) until a specified
     * color is found.
     * @param bitmap    the bitmap
     * @param start     starting point
     * @param color     color to find
     * @return pixel position coming after the starting point in the scaline order, or
     * {@link #SCANLINE_SEARCH_NOT_FOUND} if no such pixel found till the end (right-bottom bitmap
     * corner).
     */
    public IntPoint scanlineSearch(Bitmap bitmap, IntPoint start, Color color) {
        return scanlineSearchInt(bitmap.handle, start.x, start.y, color.r, color.g, color.b, color.a);
    }

    public IntPoint scanlineSearch(Bitmap source, IntPoint start, FloatColor color) {
        return scanlineSearchFloat(source.handle, start.x, start.y, color.r, color.g, color.b, color.a);
    }


    /**
     * Tests whether the GPU was already queried
     * @return `true` if the GPU was queried
     */
    public boolean isGPUQueried() {
        return isGPUQueried(handle);
    }


    /**
     * Tests whether the GPU was already queried and successfully initialized
     * @return `true` if yes
     */
    public boolean isGPUReady() {
        return isGPUReady(handle);
    }


    /**
     * Recycles GPU-managed resources that are ready to be disposed in a separate task
     */
    public void recycleGPUGarbage() {
        recycleGPUGarbage(handle);
    }


    /**
     * @return total size of RAM in bytes
     */
    public static long getTotalRAMBytes() {
        return getTotalRam();
    }

    public static final IntPoint SCANLINE_SEARCH_NOT_FOUND = new IntPoint(-1, -1);
}
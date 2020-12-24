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

package Beatmup;

import java.util.HashMap;

import Beatmup.Exceptions.CoreException;
import Beatmup.Geometry.IntPoint;
import Beatmup.Imaging.Color;
import Beatmup.Imaging.FloatColor;
import Beatmup.Imaging.PixelFormat;


/**
 * Beatmup engine context.
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

    private native float performTask(long ctx, int poolIndex, Task task) throws CoreException;
    private native int submitTask(long ctx, int poolIndex, Task task);
    private native int submitPersistentTask(long ctx, int poolIndex, Task task);
    private native void repeatTask(long ctx, int poolIndex, Task task, boolean abortCurrent);
    private native void waitForJob(long ctx, int poolIndex, int job);
    private native boolean abortJob(long ctx, int poolIndex, int job);
    private native void waitForAllJobs(long ctx, int poolIndex);
    private native boolean busy(long ctx, int poolIndex);
    private native void check(long ctx, int poolIndex) throws CoreException;

    private native long renderChessboard(long ctx, int width, int height, int cellSize, int pixelFormat);
    private native long copyBitmap(Bitmap bitmap, int newPixelFormat);
    private native IntPoint scanlineSearchInt(long bitmap, int startX, int startY, int r, int g, int b, int a);
    private native IntPoint scanlineSearchFloat(long bitmap, int startX, int startY, float r, float g, float b, float a);

    private native int maxAllowedWorkerCount(long ctx, int poolIndex);
    private native void limitWorkerCount(long ctx, int poolIndex, int count);

    private native boolean isGPUQueried(long ctx);
    private native boolean isGPUReady(long ctx);
    private native void recycleGPUGarbage(long ctx);


    /**
     * Performs a given task in the main thread pool.
     * @param task        The task to run
     * @return execution time in ms.
     * @throws CoreException if the main thread pool has unprocessed exceptions thrown by previously executed tasks.
     */
    public float performTask(Task task) throws CoreException {
        return performTask(handle, 0, task);
    }


    /**
     * Performs a given task.
     * @param task          The task to run
     * @param poolIndex     Zero-based index of the thread pool to run the task in.
     * @return execution time in ms.
     * @throws CoreException if the thread pool has unprocessed exceptions thrown by previously executed tasks.
     */
    public float performTask(Task task, int poolIndex) throws CoreException {
        return performTask(handle, poolIndex, task);
    }


    /**
     * Ensures a given task executed at least once.
     * @param task          the task
     * @param abortCurrent  if `true` and a task is running, the abort signal is sent.
     */
    public void repeatTask(Task task, boolean abortCurrent) {
        repeatTask(handle, 0, task, abortCurrent);
    }


    /**
     * Ensures a given task executed at least once in a specific thread pool.
     * @param task          The task
     * @param abortCurrent  if `true` and a task is running, the abort signal is sent.
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
    public void waitForJob(int job, int poolIndex) {
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
     * Sets maximum number of threads executing tasks in a given thread pool.
     * @param newCount      the new thread count limit
     * @param poolIndex     index of the thread pool
     */
    public void limitWorkerCount(int newCount, int poolIndex) {
        limitWorkerCount(handle, poolIndex, newCount);
    }


    /**
     * Checks if the main thread pool is doing great: rethrows exceptions occurred during tasks execution, if any.
     * If no exception is thrown, the thread pool is okay.
     * @throws CoreException occurred while running a task.
     */
    public void check() throws CoreException {
        check(handle, 0);
    }


    /**
     * Checks if a specific thread pool is doing great: rethrows exceptions occurred during tasks execution, if any.
     * If no exception is thrown, the thread pool is okay.
     * @param poolIndex         The thread pool index
     * @throws CoreException occurred while running a task.
     */
    public void check(int poolIndex) throws CoreException {
        check(handle, poolIndex);
    }

    /**
     * Creates a new context.
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
     * Renders a chessboard-like image.
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
     * Creates a copy of given bitmap.
     * @param source            the bitmap
     * @param pixelFormat       pixel format of the copy
     * @return copy of the bitmap in the given pixel format
     */
    public Bitmap copyBitmap(Bitmap source, PixelFormat pixelFormat) {
        return new Bitmap(this, copyBitmap(source, pixelFormat.ordinal()));
    }


    /**
     * Goes through a bitmap in scanline order (left to right, top to bottom) until a pixel of a given color is met.
     * @param bitmap    the bitmap to scan
     * @param color     the color value to look for
     * @param start     starting pixel position
     * @return pixel position coming after the starting point in the scaline order, or {@link #SCANLINE_SEARCH_NOT_FOUND} if no such pixel found till the end (right-bottom bitmap
     * corner).
     */
    public IntPoint scanlineSearch(Bitmap bitmap, Color color, IntPoint start) {
        return scanlineSearchInt(bitmap.handle, start.x, start.y, color.r, color.g, color.b, color.a);
    }

    public IntPoint scanlineSearch(Bitmap source, FloatColor color, IntPoint start) {
        return scanlineSearchFloat(source.handle, start.x, start.y, color.r, color.g, color.b, color.a);
    }


    /**
     * Tests whether the GPU was already queried.
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

    /**
     * Returned by {@link Beatmup.Context.scanlineSearch()} if no pixel of a specific color is found in the image.
     */
    public static final IntPoint SCANLINE_SEARCH_NOT_FOUND = new IntPoint(-1, -1);
}
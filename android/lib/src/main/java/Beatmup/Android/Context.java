package Beatmup.Android;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;

import java.io.File;
import java.io.IOException;

/**
 * Beatmup engine context for Android
 */
public class Context extends Beatmup.Context {
    protected java.lang.Object glSurface;             //!< a handle to a surface currently bound to the context, accessed from JNI layer

    private static native long newEnvironment(int numPools, String filesFolder);

    private static String getPathAsPrefix(File filesDir) {
        if (filesDir == null)
            return null;
        String path = filesDir.getAbsolutePath();
        if (!path.endsWith(File.separator))
            path = path + File.separator;
        return path;
    }


    /**
     * Context initialization
     * @param poolCount     desired number of thread pools (max number of tasks to run in parallel)
     * @param filesDir      client files folder to use for swapping
     */
    public Context(final int poolCount, File filesDir) {
        super(newEnvironment(poolCount, getPathAsPrefix(filesDir)));
        glSurface = null;
    }


    @Override
    public synchronized void dispose() {
        super.dispose();
    }
}

package Beatmup.Visual.Android;

import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.ArrayList;

import Beatmup.Context;
import Beatmup.Rendering.SceneRenderer;

/**
 * Custom OpenGL view
 */
public class BasicDisplay extends SurfaceView {
    public interface OnSizeChangeListener {
        void sizeChanged(int newWidth, int newHeight);
    }

    protected SceneRenderer renderer;
    protected ArrayList<OnSizeChangeListener> sizeChangeListeners;

    /**
     * Binds together a Beatmup context and an android surface to render on screen
     * @param context   the context
     * @param surface   the surface
     * @return `true` on success, `false` otherwise; may throw a Java exception from inside
     */
    private native boolean bindSurfaceToContext(Context context, Surface surface);


    private void makeCurrent() {
        Surface surface = getHolder().getSurface();
        if (renderer != null && surface.isValid()) {
            if (bindSurfaceToContext(renderer.getContext(), surface)) {
                renderer.render();
            }
            else
                Log.e("AndroidBasicGLDisplay", "Unable to bind the surface");
        }
    }

    private void unmakeCurrent() {
        if (renderer != null)
            if (!bindSurfaceToContext(renderer.getContext(), null))
                Log.e("AndroidBasicGLDisplay", "Unable to unbind the surface");
    }

    public BasicDisplay(final android.content.Context context, AttributeSet attrs) {
        super(context, attrs);
        sizeChangeListeners = new ArrayList<>();

        getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                makeCurrent();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                makeCurrent();
                for (OnSizeChangeListener listener : sizeChangeListeners)
                    listener.sizeChanged(width, height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                unmakeCurrent();
            }
        });
    }


    public void setRenderer(SceneRenderer newRenderer) {
        renderer = newRenderer;
        makeCurrent();
    }


    public SceneRenderer getRenderer() {
        return renderer;
    }


    public void addSizeChangeListeners(OnSizeChangeListener aListener) {
        this.sizeChangeListeners.add(aListener);
    }


    public boolean removeSizeChangeListeners(OnSizeChangeListener aListener) {
        return sizeChangeListeners.remove(aListener);
    }

}

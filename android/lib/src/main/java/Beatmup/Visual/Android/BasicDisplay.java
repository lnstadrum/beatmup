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

    public interface OnBindingListener {
        /**
         * Called before a display-to-context binding event happens.
         * The target context have to run system tasks to set up the display. If it is busy with
         * other jobs (namely persistent ones), the binding will then be blocked and UI will be
         * freezed.
         */
        void beforeBinding(boolean valid);

        /**
         * Called after a display-to-context binding event happens.
         * The target context have to run system tasks to set up the display. If it is busy with
         * other jobs (namely persistent ones), the binding will then be blocked and UI will be
         * freezed.
         */
        void afterBinding(boolean valid);

    }

    protected SceneRenderer renderer;
    protected ArrayList<OnSizeChangeListener> sizeChangeListeners;
    protected ArrayList<OnBindingListener> bindingListeners;

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
            for (OnBindingListener listener : bindingListeners)
                listener.beforeBinding(true);

            if (!bindSurfaceToContext(renderer.getContext(), surface))
                Log.e("BasicDisplay", "Unable to bind the surface");

            for (OnBindingListener listener : bindingListeners)
                listener.afterBinding(true);
        }
    }

    private void unmakeCurrent() {
        if (renderer != null) {
            for (OnBindingListener listener : bindingListeners)
                listener.beforeBinding(false);

            if (!bindSurfaceToContext(renderer.getContext(), null))
                Log.e("BasicDisplay", "Unable to unbind the surface");

            for (OnBindingListener listener : bindingListeners)
                listener.afterBinding(false);
        }
    }

    public BasicDisplay(final android.content.Context context, AttributeSet attrs) {
        super(context, attrs);
        sizeChangeListeners = new ArrayList<>();
        bindingListeners = new ArrayList<>();

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


    public void addSizeChangeListener(OnSizeChangeListener aListener) {
        this.sizeChangeListeners.add(aListener);
    }


    public boolean removeSizeChangeListener(OnSizeChangeListener aListener) {
        return sizeChangeListeners.remove(aListener);
    }


    public void addBindingListener(OnBindingListener aListener) {
        this.bindingListeners.add(aListener);
    }


    public boolean removeBindingListener(OnBindingListener aListener) {
        return bindingListeners.remove(aListener);
    }

}

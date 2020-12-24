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

package Beatmup.Visual.Android;

import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.ArrayList;

import Beatmup.Context;
import Beatmup.Exceptions.CoreException;
import Beatmup.Rendering.SceneRenderer;

/**
 * Surface displaying the rendering output of its {@link SceneRenderer}.
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
     * @throws CoreException if the main thread of the context contains exceptions occurred while processing other tasks.
     */
    private native boolean bindSurfaceToContext(Context context, Surface surface) throws CoreException;

    /**
     * Binds the display as current in the renderer.
     * The renderer output is then shown on the display (after the next rendering session).
     */
    private void makeCurrent()  {
        Surface surface = getHolder().getSurface();
        if (renderer != null && surface.isValid()) {
            for (OnBindingListener listener : bindingListeners)
                listener.beforeBinding(true);

            try {
                if (!bindSurfaceToContext(renderer.getContext(), surface))
                    Log.e("BasicDisplay", "Unable to bind the surface");
            } catch (CoreException e) {
                e.printStackTrace();
            }

            for (OnBindingListener listener : bindingListeners)
                listener.afterBinding(true);
        }
    }

    /**
     * Unbinds the display as current from renderer.
     */
    private void unmakeCurrent() {
        if (renderer != null) {
            for (OnBindingListener listener : bindingListeners)
                listener.beforeBinding(false);

            try {
                if (!bindSurfaceToContext(renderer.getContext(), null))
                    Log.e("BasicDisplay", "Unable to unbind the surface");
            } catch (CoreException e) {
                e.printStackTrace();
            }

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

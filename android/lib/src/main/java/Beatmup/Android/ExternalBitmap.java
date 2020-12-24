/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

package Beatmup.Android;

import android.graphics.SurfaceTexture;

import Beatmup.Bitmap;
import Beatmup.Context;

/**
 * A bitmap coming from an external source (camera or decoder) in a form of an OpenGL texture.
 */
public class ExternalBitmap extends Bitmap {
    private native static long newExternalImage(Context context);
    private native void bind(long handle);
    private native void notifyUpdate(long handle, int width, int height);
    private SurfaceTexture surfaceTexture;

    ExternalBitmap(Context context) {
        super(context, newExternalImage(context));
        bind(handle);
    }

    void notifyUpdate(int width, int height) {
        notifyUpdate(handle, width, height);
    }

    SurfaceTexture getSurfaceTexture() {
        return surfaceTexture;
    }
}

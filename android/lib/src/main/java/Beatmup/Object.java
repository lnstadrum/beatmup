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

/**
 * Base class for objects natively managed by Beatmup.
 */
public class Object {
    protected long handle;      //!< pointer to the native object

    private native void disposeNative();

    protected Object(long handle) {
        this.handle = handle;
    }

    /**
     * Destroys the native object.
     * After the native object is destroyed, its Java counterpart likely becomes unusable.
     */
    protected synchronized void dispose() {
        disposeNative();
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        return (obj instanceof Beatmup.Object) && (handle == ((Beatmup.Object) obj).handle);
    }

    @Override
    protected void finalize() throws Throwable {
        dispose();
        super.finalize();
    }
}

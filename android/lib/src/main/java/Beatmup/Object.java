package Beatmup;

/**
 * Base class for any Java class whose instances interact with internal engine functions
 */
public class Object {
    protected long handle;

    private native void disposeNative();

    protected Object(long handle) {
        this.handle = handle;
    }

    public synchronized void dispose() {
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

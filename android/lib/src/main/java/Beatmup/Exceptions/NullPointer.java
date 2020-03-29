package Beatmup.Exceptions;

/**
 * Exception thrown from native code when a pointer is not initialized
 */
public class NullPointer extends NullPointerException {
    public NullPointer(String message) {
        super(message);
    }
}

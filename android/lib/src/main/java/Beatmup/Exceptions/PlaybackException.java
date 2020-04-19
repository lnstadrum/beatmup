package Beatmup.Exceptions;

/**
 * Thrown by the playback from native code, mainly if there is something wrong with the playback mode
 */
public class PlaybackException extends CoreException {
    public PlaybackException(String message) {
        super(message);
    }
}

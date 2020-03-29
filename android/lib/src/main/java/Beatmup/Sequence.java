package Beatmup;

/**
 * An editable sequence defined in time
 */
public class Sequence extends Beatmup.Object {

    private native long copy(long handle, int start, int end);
    private native void insert(long handle, long anotherHandle, int time);
    private native void remove(long handle, int start, int end);
    private native void shrink(long handle, int start, int end);

    protected Sequence(long handle) {
        super(handle);
    }

    /**
     * Extracts a subsequence by creating a copy
     * @param start     the copied part start time
     * @param end       the copied part end time
     * @return new sequence containing the copy
     */
    public Sequence copy(int start, int end) {
        return new Sequence(copy(handle, start, end));
    }

    /**
     * Inserts another sequence in the current one
     * @param sequence      the new data to insert
     * @param time          time moment where the new sequence will be inserted
     */
    public void insert(Sequence sequence, int time) {
        insert(handle, sequence.handle, time);
    }

    /**
     * Removes out a part from the sequence
     * @param start     the start time moment to remove data from
     * @param end       the end time moment
     */
    public void remove(int start, int end) {
        remove(handle, start, end);
    }
}

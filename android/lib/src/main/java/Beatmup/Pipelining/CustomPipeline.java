package Beatmup.Pipelining;

import Beatmup.*;

/**
 * Sequential task execution
 */
public class CustomPipeline extends Task {
    private native int getTaskCount(long handle);
    private native TaskHolder getTask(long handle, int index);
    private native int getTaskIndex(long handle, long taskHolderHandle);
    private native long addTask(long handle, TaskHolder holder, Task task);
    private native long insertTask(long handle, TaskHolder holder, Task task, long succeedingTaskHolderHandle);
    private native boolean removeTask(long handle, long taskHolderHandle);
    private native void measure(long handle);

    protected CustomPipeline(Context context, long handle) {
        super(context, handle);
    }

    /**
     * @return number of tasks in the pipeline
     */
    public int getTaskCount() {
        return getTaskCount(handle);
    }

    /**
     * Retrieves a task by its index
     * @param index     the index (its number in the task list)
     * @return the task holder
     */
    public TaskHolder getTask(int index) {
        return getTask(handle, index);
    }

    /**
     * Retrieves task number in the list of a given task holder
     * @param taskHolder    the holder to find
     * @return its number in the sequence or -1 if not present
     */
    public int getTaskIndex(TaskHolder taskHolder) {
        return getTaskIndex(handle, taskHolder.handle);
    }

    /**
     * Puts a new task into the end of the task list of the pipeline
     * @param task      the new task
     * @return task holder wrapping the new task
     */
    public TaskHolder addTask(Task task) {
        TaskHolder holder = new TaskHolder(task);
        holder.handle = addTask(handle, holder, task);
        return holder;
    }

    /**
     * Puts a new task at a specified place in the task list of the pipeline
     * @param task          the new task to put in the pipeline
     * @param succeeding    the task holder before which the new task will be inserted
     * @return task holder wrapping the new task
     */
    public TaskHolder insertTask(Task task, TaskHolder succeeding) {
        TaskHolder holder = new TaskHolder(task);
        holder.handle = insertTask(handle, holder, task, succeeding.handle);
        return holder;
    }

    /**
     * Removes a task from the task list of the pipeline
     * @param taskHolder        the task to remove
     * @return `true` on success, i.e., if the task actually was in the list; `false` otherwise
     */
    public boolean removeTask(TaskHolder taskHolder) {
        return removeTask(handle, taskHolder.handle);
    }

    /**
     * Determines execution mode (CPU or GPU) and number of threads
     */
    public void measure() {
        measure(handle);
    }

    public void repeat(boolean abortCurrent) {
        context.repeatTask(this, abortCurrent);
    }
}

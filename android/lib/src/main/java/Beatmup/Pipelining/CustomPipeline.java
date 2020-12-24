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

package Beatmup.Pipelining;

import Beatmup.*;

/**
 * Custom pipeline: a sequence of tasks to be executed as a whole.
 * Acts as an AbstractTask. Built by adding tasks one by one and calling measure() at the end.
 */
public class CustomPipeline extends Task {

    // native methods
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
     * Retrieves a task by its index.
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
     * @param task      The task to insert
     * @param before    TaskHolder specifying position of the task that will follow the newly inserted task
     * @return TaskHolder with the newly inserted task.
     */
    public TaskHolder insertTask(Task task, TaskHolder before) {
        TaskHolder holder = new TaskHolder(task);
        holder.handle = insertTask(handle, holder, task, before.handle);
        return holder;
    }

    /**
     * Removes a task from the pipeline.
     * @param taskHolder    The task to remove
     * @return `true` on success, `false` if this TaskHolder is not in the pipeline.
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
}

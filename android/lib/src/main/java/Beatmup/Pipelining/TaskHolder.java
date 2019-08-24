package Beatmup.Pipelining;

import Beatmup.Task;
import Beatmup.Object;

/**
 * Task container for pipelining
 */
public class TaskHolder {
    private Task task;
    protected long handle;

    protected TaskHolder(Task task) {
        this.task = task;
        this.handle = 0;
    }

    public Task getTask() {
        return task;
    }
}

package Beatmup;

import Beatmup.Context;

/**
 * Abstract task
 */
public class Task extends Object {
    protected Context context;

    protected Task(Context context, long handle) {
        super(handle);
        this.context = context;
    }

    public Context getContext() {
        return context;
    }

    /**
     * Runs the task
     * @return execution time in ms
     */
    public float execute() {
        return context.performTask(this);
    }
}

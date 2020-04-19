package Beatmup.Android;

import android.app.Activity;

/**
 * UI task that is iterated on each update from the user and finished when the user is happy on cancels the command
 */
public abstract class UITask {
    private Activity activity;
    private final Task task;
    private Thread thread;

    private final Runnable
            onStart,
            onIterationStart,
            onIterationDone,
            onDone;

    private boolean
            rewind,         //!< if `true`, the cycle is asked to restart
            running,        //!< if `true`, the task is going on
            cancelled;      //!< if `true`, the task is completely cancelled


    private class Task implements Runnable {

        @Override
        public void run() {
            // run onStart in main thread
            synchronized (onStart) {
                activity.runOnUiThread(onStart);
                try {
                    onStart.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            // task cycle
            while (true) {
                synchronized (task) {
                    // check if a new iteration is actually needed
                    if (!rewind || cancelled) {
                        running = false;
                        activity.runOnUiThread(onDone);
                        return;
                    }
                    rewind = false;
                }

                // run onIterationStart in main thread
                synchronized (onIterationStart) {
                    activity.runOnUiThread(onIterationStart);
                    try {
                        onIterationStart.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                // do the job
                process();

                if (!rewind && !cancelled) {
                    // run onIterationDone in main thread
                    synchronized (onIterationDone) {
                        activity.runOnUiThread(onIterationDone);
                        try {
                            onIterationDone.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        }
    }

    public UITask(Activity activity) {
        this.activity = activity;
        task = new Task();

        onStart = new Runnable() {
            @Override
            public void run() {
                onStart();
                synchronized (this) {
                    this.notify();
                }
            }
        };

        onIterationStart = new Runnable() {
            @Override
            public void run() {
                onIterationStart();
                synchronized (this) {
                    this.notify();
                }
            }
        };

        onIterationDone = new Runnable() {
            @Override
            public void run() {
                onIterationDone();
                synchronized (this) {
                    this.notify();
                }
            }
        };

        onDone = new Runnable() {
            @Override
            public void run() {
                onDone(cancelled);
            }
        };
    }

    /**
     * Starts or rewinds the task
     */
    public void start() {
        synchronized (task) {
            rewind = true;
            if (!running) {
                running = true;
                thread = new Thread(task);
                thread.start();
            }
        }
    }

    /**
     * Cancels the task waiting it to finish
     */
    public void cancel() {
        Thread aThreadToWait = null;
        synchronized (task) {
            cancelled = true;
            if (thread != null)
                aThreadToWait = thread;
        }
        try {
            if (aThreadToWait != null)
                aThreadToWait.join();
        } catch (InterruptedException e) {
            // do nothing
        }
    }

    /**
     * Called once before the task cycle. To use to prepare UI (lock controls)
     */
    public void onStart() {
        // nothing to do by default
    }

    /**
     * Called on iteration start
     */
    public void onIterationStart() {
        // nothing to do by default
    }

    /**
     * Called after iteration successfully finished
     */
    public void onIterationDone() {
        // nothing to do by default
    }

    /**
     * Called once after the task cycle. To use to prepare UI (unlock controls)
     * @param cancelled     if `true`,
     */
    public void onDone(boolean cancelled) {
        // nothing to do by default
    }

    public abstract void process();

    /**
     * Signals whether the current iteration is cancelled
     * @return `true` if the current iteration must finish as soon as possible
     */
    protected boolean iterationCancelled() {
        return rewind;
    }

}

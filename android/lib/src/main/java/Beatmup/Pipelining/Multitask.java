package Beatmup.Pipelining;

import Beatmup.*;

/**
 * Conditional multiple tasks execution
 *
 * Beatmup offers a number of tools allowing to pipeline several tasks into a single one. This
 * technique is particularly useful for designing complex multi-stage image processing routines.
 *
 * Multitask is the simplest such tool. It allows to concatenate different tasks into a linear
 * conveyor and run them all or selectively. To handle this selection, each task is associated with
 * a repetition policy specifying the conditions whether this given task is executed or ignored
 * when the pipeline is running.
 *
 * Specifically, there are two extreme modes that force the task execution every time
 * (REPEAT_ALWAYS) or its unconditional skipping (IGNORE_ALWAYS) and two more sophisticated modes
 * with the following behavior:
 *  - IGNORE_IF_UPTODATE skips the task if no tasks were executed among the preceding this one in
 *    the pipeline;
 *  - REPEAT_UPDATE forces task repetition one time on next run and just after switches the
 *    repetition policy to IGNORE_IF_UPTODATE.
 */
public class Multitask extends CustomPipeline {
    private static native long newMultitask(Context context);
    private native int getRepetitionPolicy(long handle, long taskHolderHandle);
    private native void setRepetitionPolicy(long handle, long taskHolderHandle, int policy);

    /**
     * Specification of conditions whether a specified task in the sequence should be executed
     */
    public enum RepetitionPolicy {
        REPEAT_ALWAYS,          //!< execute the task unconditionally on each run
        REPEAT_UPDATE,          //!< execute the task one time then switch to IGNORE_IF_UPTODATE
        IGNORE_IF_UPTODATE,     //!< do not execute the task if no preceding tasks are run
        IGNORE_ALWAYS           //!< do not execute the task
    }

    public Multitask(Context context) {
        super(context, newMultitask(context));
    }

    /**
     * Puts a new task into the end of the task list of the pipeline
     * @param task      the new task
     * @param policy    the new task repetition policy
     * @return task holder wrapping the new task
     */
    public TaskHolder addTask(Task task, RepetitionPolicy policy) {
        TaskHolder holder = addTask(task);
        setRepetitionPolicy(handle, holder.handle, policy.ordinal());
        return holder;
    }

    /**
     * Retrieves repetition policy of a specified task
     * @param taskHolder    the task holder
     * @return the task repetition policy
     */
    public RepetitionPolicy getRepetitionPolicy(TaskHolder taskHolder) {
        return RepetitionPolicy.values()[ getRepetitionPolicy(handle, taskHolder.handle) ];
    }

    /**
     * Sets up new repetition policy of a specified task
     * @param taskHolder    the task holder
     * @param policy        the new policy
     */
    public void setRepetitionPolicy(TaskHolder taskHolder, RepetitionPolicy policy) {
        setRepetitionPolicy(handle, taskHolder.handle, policy.ordinal());
    }
}

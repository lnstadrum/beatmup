/*
    Conditional multiple task executor
*/

#pragma once
#include "custom_pipeline.h"

namespace Beatmup {
    class Multitask : public CustomPipeline {
    private:
        std::mutex policyAccess;	//!< access control to modify repetition policies

    protected:
        TaskHolder* createTaskHolder(AbstractTask &task);
        bool route(TaskRouter & router);

    public:
        /**
     	 * Specification of conditions whether a specified task in the sequence should be executed
         */
        enum class RepetitionPolicy {
            REPEAT_ALWAYS,          //!< execute the task unconditionally on each run
            REPEAT_UPDATE,          //!< execute the task one time then switch to IGNORE_IF_UPTODATE
            IGNORE_IF_UPTODATE,     //!< do not execute the task if no preceding tasks are run
            IGNORE_ALWAYS           //!< do not execute the task
        };

        Multitask();
        RepetitionPolicy getRepetitionPolicy(const TaskHolder& task);

        /**
            Switches repetition policy of a task. If the pipeline is processing now, it is on the user
            responsibility to abort and restart it, if the policy change is critical.
        */
        void setRepetitionPolicy(TaskHolder& task, RepetitionPolicy policy);
    };
}
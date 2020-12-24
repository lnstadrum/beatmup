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

#pragma once
#include "custom_pipeline.h"

namespace Beatmup {
    /** \page ProgrammingModel
        \section secMultistage Multi-stage processing
        The thread pools make easy running several tasks one after the other. However, when the same pattern of tasks is needed to be run repeatedly,
        %Beatmup offers a technique to put multiple tasks together into a single compound task, Beatmup::Multitask. This enables designing complex
        application-specific processing pipelines.

        A multitask is a pipeline of tasks processing some data in a multi-stage fashion. It can simply host multiple tasks and run them in order,
        without explicitly submitting them into a thread pool. It also implements a set of repetition policies allowing to skip some stages at the
        beginning of the pipeline, if no changes is made to the input data and parameters with respect to the previous run, for example.
    */

    /**
        Conditional multiple tasks execution.

        Multitask is a technique to build complex multi-stage image processing pipelines. It allows to concatenate different tasks into a linear
        conveyor and run them all or selectively. To handle this selection, each task is associated with a repetition policy specifying the
        conditions whether this given task is executed or ignored when the pipeline is running.

        Specifically, it implements two extreme modes that force the task execution every time (Multitask::RepetitionPolicy::REPEAT_ALWAYS) or its
        unconditional skipping (Multitask::RepetitionPolicy::IGNORE_ALWAYS) and two more sophisticated modes:
         - Multitask::RepetitionPolicy::IGNORE_IF_UPTODATE skips the task if no tasks were executed among the ones coming before the current task
           in the pipeline;
         - Multitask::RepetitionPolicy::REPEAT_UPDATE forces task repetition one time on next run and just after switches the repetition policy to
           IGNORE_IF_UPTODATE.
    */
    class Multitask : public CustomPipeline {
    private:
        std::mutex policyAccess;	//!< access control to modify repetition policies

    protected:
        TaskHolder* createTaskHolder(AbstractTask &task);
        bool route(TaskRouter & router);

    public:
        /**
         * Determines when a specific task in the sequence is run when the whole sequence is invoked
         */
        enum class RepetitionPolicy {
            REPEAT_ALWAYS,          //!< execute the task unconditionally on each run
            REPEAT_UPDATE,          //!< execute the task one time then switch to IGNORE_IF_UPTODATE
            IGNORE_IF_UPTODATE,     //!< do not execute the task if no preceding tasks are run
            IGNORE_ALWAYS           //!< do not execute the task
        };

        Multitask();

        /**
            \return repetition policy of a specific task in the pipeline.
        */
        RepetitionPolicy getRepetitionPolicy(const TaskHolder&);

        /**
            Sets repetition policy of a task. If the pipeline is processing at the moment of the call, it is the application responsibility to abort
            and restart it, if the policy change needs to be applied immediately.
            \param taskHolder   TaskHolder of a task to apply the policy to
            \param policy       The new policy
        */
        void setRepetitionPolicy(TaskHolder& taskHolder, RepetitionPolicy policy);
    };
}
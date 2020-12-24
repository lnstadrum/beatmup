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
#include "basic_types.h"
#include <condition_variable>
#include <thread>

namespace Beatmup {
    /** \page ProgrammingModel

        \section secTasks Tasks
        A task (instance of AbstractTask) is an isolated elementary processing operation. It can run in parallel in multiple threads for speed, on
        CPU and/or GPU.

        \subsection ssecStruct Task structure
        The tasks are not intended to contain any user code. If you need a specific processing function to be implemented, much likely you need to
        subclass AbstractTask.

        In short, an AbstractTask has three main phases:
         - a phase "before processing" run in a single thread to perform the necessary prepration,
         - the main processing phase run in as many threads as possible, actually performing computatively intensive operations,
         - a phase "after processing" to clean up.
        The number of threads running the main phase is minimum of two values: the total number of threads in the thread pool the task is submitted
        to, and the maximum allowed number of threads given by the task itself.

        A detailed description is available in AbstractTask documentation.

        \subsection ssecExceptions Exceptions handling
        The tasks can throw exceptions. If this happens, the thread pool that is in charge of running the failing task stores the exception internally
        and rethows it back to the application code, when the latter calls Context::check() function.

        It is recommended to call Context::check() in a timely manner to process exceptions produced by tasks.

        \subsection ssecJobs Jobs
        When a task is submitted to a thread pool using Context::submitTask() function, it produces a job. A job is just a ticket number in
        the queue of the corresponding thread pool. Context functions take it to check the task status or cancel it. In this way, the same task can
        be submitted several times to the same thread pool, producing several different jobs, and will then be run several times.

        If the asynchronous behavior is not needed, a task can be run in a blocking call to Context::performTask(). This hides the mechanics of jobs
        from the user and just runs a given task.

        \subsection ssecPersistent Persistent tasks
        Usually, once a task is completed, it is dropped from the thread pool queue. This is referred to as the "normal mode", differently to
        the "persistent mode" in which the task is getting repeated until it decides to quit itself. This is convenient for rendering and playback
        tasks consuming signals from external sources, that are still run in a granular fashion (by frame or signal buffer) but persist until
        the data is fully consumed.

        Context::submitPersistentTask() produces a persistent job for a specific task.
    */

    typedef unsigned char PoolIndex;					//!< number of tread pools or a pool index
    typedef unsigned char ThreadIndex;					//!< number of threads / thread index
    typedef int Job;

    static const ThreadIndex MAX_THREAD_INDEX = 255;	//!< maximum possible thread index value

    class GraphicPipeline;
    class TaskThread;

    /**
     * Task: an operation that can be executed by multiple threads in parallel.
     * Before executing the task, ThreadPool queries the task concerning
     *  - the device (CPU or GPU) the task will be run on, by calling getUsedDevices(),
     *  - the maximum number of threads the task may be run in, by calling getMaxThreads().
     * By default the tasks run on CPU in a single thread. This behavior can be overridden in subclasses.
     * The task has three main stages:
     *  - beforeProcessing() to prepare the operation. It is run in a single thread.
     *  - processOnGPU() and process() to execute the operation.
     *     - processOnGPU() is called from a single thread having access to the GPU.
     *     - process() is potentially called from multiple threads to run the operation on CPU.
     *       The number of theads is not greatr than what was returned by getMaxThreads().
     *  - afterProcessing() to tear down the operation. It is run in a single thread as well.
     */
    class AbstractTask : public Object {
    public:
        /**
            Specifies which device (CPU and/or GPU) is used to run the task.
        */
        enum class TaskDeviceRequirement {
            CPU_ONLY,       //!< this task does not use GPU
            GPU_OR_CPU,     //!< this task uses GPU if it is available, but CPU fallback is possible
            GPU_ONLY        //!< this task requires GPU, otherwise it cannot run
        };

        /**
            Instruction called before the task is executed.
            \param threadCount    Number of threads used to perform the task
            \param target         Device used to perform the task
            \param gpu            A graphic pipeline instance; may be null.
        */
        virtual void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);

        /**
            Instruction called after the task is executed.
            \param threadCount    Number of threads used to perform the task
            \param gpu            GPU to be used to execute the task; may be null.
            \param aborted        `true` if the task was aborted
        */
        virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);

        /**
            Executes the task on CPU within a given thread. Generally called by multiple threads.
            \param thread	associated task execution context
            \returns `true` if the execution is finished correctly, `false` otherwise
        */
        virtual bool process(TaskThread& thread) = 0;

        /**
            Executes the task on GPU.
            \param gpu       graphic pipeline instance
            \param thread    associated task execution context
            \returns `true` if the execution is finished correctly, `false` otherwise
        */
        virtual bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);

        /**
            Communicates devices (CPU and/or GPU) the task is run on.
        */
        virtual TaskDeviceRequirement getUsedDevices() const;

        /**
            Gives the upper limint on the number of threads the task may be performed by.
            The actual number of threads running a specific task may be less or equal to the returned value,
            depending on the number of workers in ThreadPool running the task.
        */
        virtual ThreadIndex getMaxThreads() const;

        /**
            Valid thread count from a given integer value
        */
        static ThreadIndex validThreadCount(int number);
    };


    /**
        Thread executing tasks
    */
    class TaskThread {
        TaskThread(const TaskThread&) = delete;
    protected:
        ThreadIndex index;			//!< current thread index

        inline TaskThread(ThreadIndex index): index(index) {}

    public:
        /**
            \return number of this thread.
        */
        inline ThreadIndex currentThread() const {
            return index;
        }

        /**
            \return `true` if this thread is the managing thread
        */
        inline bool isManaging() const {
            return index == 0;
        }

        /**
            \return overall number of threads working on the current task.
        */
        virtual ThreadIndex numThreads() const = 0;

        /**
            Returns `true` if the task is asked to stop from outside.
        */
        virtual bool isTaskAborted() const = 0;

        /**
            Blocks until all the other threads running the same task reach the same point.
            "The same point" is the same number of calls to synchronize().
            This function does not throw any exception to be caught in the task code. However, if a thread running the task has failed, this function throws
            an exception the thread pool takes care of. This ensures a valid thread pool state and avoids dead locks when a task fails to proceed.
        */
        virtual void synchronize() = 0;
    };

}

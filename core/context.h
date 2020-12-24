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
#include "parallelism.h"
#include <string>


namespace Beatmup {

    /** \page ProgrammingModel Programming model
        %Beatmup is thought as a toolset for building efficient signal and image processing pipelines.

        This page covers briefly few main concepts of a fairly simple programming model used in %Beatmup: *contexts*, *thread pools*, *tasks*, *jobs*
        and *bitmaps*.

        \section secContext Context and thread pools
        A Context instance mainly contains one or more thread pools that can execute processing actions (tasks). At least one Context instance (and
        quite often just one) is required to do anything in %Beatmup.

        A thread pool is a bunch of threads and a queue of tasks. Tasks are submitted by the application in a pool and are executed in order.
        A given thread pool can run only one task at a time, but it does so in multiple threads in parallel for speed.

        Thread pools work asynchronously with respect to the caller code: the tasks can be submitted by the application in a non-blocking call,
        straight from a user interface managing thread for example. Context exposes necessary API entries to check whether a specific task is
        completed or still waiting in the queue, to cancel a submitted task, to check exceptions thrown during task execution, etc.

        By default, when a thread pool is created, the number of threads it hosts is inferred from the hardware concurrency: typically, it is equal
        to the number of logical CPU cores. This setting is likely to provide the best performance for computationally intensive tasks.
        The number of threads in a pool can be further adjusted by calling Context::limitWorkerCount().
    */

    namespace GL {
        class RecycleBin;
    }

    class AbstractBitmap;

    /**
        Basic class: task and memory management, any kind of static data
    */
    class Context : public Object {
        Context(const Context&) = delete;	//!< disabling copying constructor
    private:
        class Impl;
        Impl* impl;
        GL::RecycleBin* recycleBin;                  //!< stores GPU garbage: resources managed by GPU and might be freed in the managing thread only

    public:
        static const PoolIndex DEFAULT_POOL = 0;

        /**
            An event listener (bunch of callbacks)
        */
        class EventListener {
        public:
            virtual ~EventListener() {}

            /**
                Called when a new worker thread is created
            */
            virtual void threadCreated(PoolIndex pool) = 0;

            /**
                Called when a worker thread finished
            */
            virtual void threadTerminating(PoolIndex pool) = 0;

            /**
                Called when a task is successfully finished.
                \param[in] pool     The thread pool the task was run in
                \param[in] task     The task
                \param[in] aborted  If `true`, the task was aborted from outside
                \returns `true` if the task is asked to be executed again. Note that even if `false` is returned, a repetition might be asked from outside.
            */
            virtual bool taskDone(PoolIndex pool, AbstractTask& task, bool aborted) = 0;

            /**
                Called when a task fails.
                \param[in] pool     The thread pool the task was run in
                \param[in] task     The task
                \param[in] ex       Exception caught when the task is failed
            */
            virtual void taskFail(PoolIndex pool, AbstractTask& task, const std::exception& ex) = 0;

            /**
                Called when GPU initialization failed.
                \param[in] pool     The thread pool the failure occurred in
                \param[in] ex       Exception caught
            */
            virtual void gpuInitFail(PoolIndex pool, const std::exception& ex) = 0;
        };

        Context();
        Context(const PoolIndex numThreadPools);
        ~Context();

        /**
            Performs a given task.
            \param task				The task
            \param pool				A thread pool to run the task in
            \returns task execution time
        */
        float performTask(AbstractTask& task, const PoolIndex pool = DEFAULT_POOL);

        /**
            Ensures a given task executed at least once.
            \param task				The task
            \param abortCurrent		if `true` and the same task is currently running, the abort signal is sent.
         	\param pool				A thread pool to run the task in
        */
        void repeatTask(AbstractTask& task, bool abortCurrent, const PoolIndex pool = DEFAULT_POOL);

        /**
            Adds a new task to the jobs queue.
            \param task				The task
            \param pool				A thread pool to run the task in
         */
        Job submitTask(AbstractTask& task, const PoolIndex pool = DEFAULT_POOL);

        /**
            Adds a new persistent task to the jobs queue.
            Persistent task is repeated until it decides itself to quit.
            \param task				The task
            \param pool				A thread pool to run the task in
         */
        Job submitPersistentTask(AbstractTask& task, const PoolIndex pool = DEFAULT_POOL);

        /**
            Waits until a given job finishes.
            \param job          The job
            \param pool         Pool index
        */
        void waitForJob(Job job, const PoolIndex pool = DEFAULT_POOL);

        /**
            Aborts a given submitted job.
            \param job     The job
            \param pool    Pool index
            \return `true` if the job was interrupted while running.
        */
        bool abortJob(Job job, const PoolIndex pool = DEFAULT_POOL);

        /**
            Blocks until all the submitted jobs are executed.
            \param pool    Pool index
        */
        void wait(const PoolIndex pool = DEFAULT_POOL);

        /**
            Queries whether a given thread pool is busy with a task.
            \param pool			The thread pool to query
            \return `true` if the thread pool is running a task, `false` otherwise
        */
        bool busy(const PoolIndex pool = DEFAULT_POOL);

        /**
            Checks if a specific thread pool is doing great: rethrows exceptions occurred during tasks execution, if any.
            \param pool     The thread pool to query
        */
        void check(const PoolIndex pool = DEFAULT_POOL);

        /**
            \returns maximum number of working threads per task in a given pool.
        */
        const ThreadIndex maxAllowedWorkerCount(const PoolIndex pool = DEFAULT_POOL) const;

        /**
            Limits maximum number of threads (workers) when performing tasks in a given pool
            \param maxValue		Number limiting the worker count
            \param pool			The thread pool
            \returns new maximum limit.
        */
        void limitWorkerCount(ThreadIndex maxValue, const PoolIndex pool = DEFAULT_POOL);

        /**
            Installs new event listener
        */
        void setEventListener(EventListener* eventListener);

        /**
            Returns current event listener (or NULL)
        */
        EventListener* getEventListener() const;

        /**
            \return `true` if GPU was queried
        */
        bool isGpuQueried() const;

        /**
            \return `true` if GPU was queried and ready to use
        */
        bool isGpuReady() const;

        /**
            \brief Initializes GPU within a given Context if not yet (takes no effect if it already is).
            GPU initialization may take some time and is done when a first task using GPU is being run. Warming up
            the GPU is useful to avoid the app get stucked for some time when it launches its first task on GPU.
        */
        void warmUpGpu();

        /**
            \brief Initializes the GPU if not yet and queries information about it.
            \param[out] vendor      GPU vendor string.
            \param[out] renderer    renderer string.
            \return `true` if a GPU is found.
        */
        bool queryGpuInfo(std::string& vendor, std::string& renderer);

        /**
            \internal
            \return `true` if invoked from the context managing thread
        */
        bool isManagingThread() const;

        /**
            \return GPU recycle bin to store GPU resources that can be freed only within a GPU-aware thread
        */
        GL::RecycleBin* getGpuRecycleBin() const;

        /**
            Context comparaison operator
            Two different instances of contexts are basically never identical; returning `true` only if the two point
            to the same instance.
        */
        inline bool operator==(const Context& context) const {
            return this == &context;
        }
    };
};

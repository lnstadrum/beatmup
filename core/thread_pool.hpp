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
#include "parallelism.h"
#include <deque>


namespace Beatmup {
    class ThreadPool;
}


/**
    A pool of threads running tasks
    ThreadPool runs AbstractTasks, possibly in multiple threads. Every time a task is submitted to the thread pool, it
    is scheduled for execution and gets attributed a corresponding Job number. Jobs are used to cancel the scheduled
    runs and track whether they have been conducted or not.
    Exceptions thrown during the tasks execution in different threads are rethrown in the caller threads.
*/
class Beatmup::ThreadPool {

    class TaskThreadImpl : public TaskThread {
    private:
        inline void threadFunc() {
            index == 0 ? pool.managingThreadFunc(*this) : pool.workerThreadFunc(*this);
        }

    public:
        inline TaskThreadImpl(ThreadIndex index, ThreadPool& pool) :
            TaskThread(index),
            pool(pool), index(index), isRunning(false), isTerminating(false),
            internalThread(&TaskThreadImpl::threadFunc, this)
        {}

        virtual inline ~TaskThreadImpl() {}

        ThreadIndex numThreads() const {
            return pool.currentWorkerCount;
        }

        bool isTaskAborted() const {
            return pool.abortExternally;
        }

        void synchronize() {
            if (numThreads() > 1)
                pool.synchronizeThread(*this);
        }

        ThreadPool& pool;
        ThreadIndex index;              //!< current thread index
        bool isRunning;                 //!< if not, the thread sleeps
        bool isTerminating;             //!< if `true`, the thread is requested to terminate
        std::thread internalThread;     //!< worker thread
    };


public:
    /**
     * Way how to the task should be run in the pool
     */
    enum class TaskExecutionMode {
        NORMAL,         //!< normal task, should be run it once
        PERSISTENT      //!< persistent task, run process() until it returns `false`
    };

    /**
        Event listener class
    */
    class EventListener {
    public:
        /**
            Callback function called when a new worker thread is created
            \param pool         Thread pool number in the context
         */
        virtual inline void threadCreated(PoolIndex pool) {};

        /**
            Callback function called when a worker thread is terminating
            \param pool         Thread pool number in the context
         */
        virtual inline void threadTerminating(PoolIndex pool) {};

        /**
            Callback function called when a task is done or cancelled; if returns `true`, the task will be repeated
            \param pool         Thread pool number in the context
            \param task         The task
            \param aborted      If `true`, the task was aborted
         */
        virtual inline bool taskDone(PoolIndex pool, AbstractTask& task, bool aborted) {
            return false;
        }

        /**
            Callback function called when an exception is thrown
            \param pool         Thread pool number in the context
            \param task         The task that was running when the exception was thrown
            \param exPtr        Exception pointer
         */
        virtual inline void taskFail(PoolIndex pool, AbstractTask& task, const std::exception_ptr exPtr) {};

        /**
            Callback function called when the GPU cannot start
            \param pool         Thread pool number in the context
            \param exPtr        Exception pointer; points to the exception instance occurred when starting up the GPU
         */
        virtual inline void gpuInitFail(PoolIndex pool, const std::exception_ptr exPtr) {};
    };

private:
    ThreadPool(const ThreadPool&) = delete;

    /**
        \internal
        Thrown in a thread when detecting that another thread failed while runinning a task, causing the current thread to stop.
    */
    class AnotherThreadFailed : public std::exception {
    public:
        AnotherThreadFailed() {}
    };

    /**
        Managing (#0) worker thread function
    */
    inline void managingThreadFunc(TaskThreadImpl& thread) {
        eventListener.threadCreated(myIndex);

        // locks
        std::unique_lock<std::mutex> lock(jobsAccess, std::defer_lock);
        std::unique_lock<std::mutex> workersLock(workersAccess, std::defer_lock);

        while (!thread.isTerminating) {
            // wait while a task is got
            lock.lock();
            while (jobs.empty() && !thread.isTerminating)
                jobsCvar.wait(lock);

            if (thread.isTerminating) {
                lock.unlock();
                break;
            }

            // fetch a task
            syncHitsCount = 0;
            syncHitsBound = 0;
            abortExternally = abortInternally = false;
            failFlag = false;
            repeatFlag = false;
            if (!jobs.empty()) {
                currentJob = jobs.front();
                currentWorkerCount = remainingWorkers = std::min(currentJob.task->getMaxThreads(), threadCount);
            }
            else
                currentJob.task = nullptr;

            // release queue access
            lock.unlock();

            // test execution mode
            AbstractTask::TaskDeviceRequirement exTarget;
            bool useGpuForCurrentTask = false;
            if (currentJob.task) {
                exTarget = currentJob.task->getUsedDevices();

                if (exTarget != AbstractTask::TaskDeviceRequirement::CPU_ONLY && myIndex == 0) {
                    // test GPU if not yet
                    if (!isGpuTested) {
                        try {
                            gpu = new GraphicPipeline();
                        }
                        catch (...) {
                            eventListener.gpuInitFail(myIndex, std::current_exception());
                            std::lock_guard<std::mutex> lock(exceptionsAccess);
                            exceptions.push_back(std::current_exception());
                            gpu = nullptr;
                        }
                        isGpuTested = true;
                    }

                    useGpuForCurrentTask = (gpu != nullptr);
                }

                // run beforeProcessing
                try {
                    if (!useGpuForCurrentTask && exTarget == AbstractTask::TaskDeviceRequirement::GPU_ONLY)
                        throw Beatmup::RuntimeError(
                            myIndex == 0 ?
                            "A task requires GPU, but GPU init is failed" :
                            "A task requiring GPU may only be run in the main pool"
                        );
                    currentJob.task->beforeProcessing(currentWorkerCount, useGpuForCurrentTask ? ProcessingTarget::GPU : ProcessingTarget::CPU, gpu);
                }
                catch (...) {
                    eventListener.taskFail(myIndex, *currentJob.task, std::current_exception());
                    std::lock_guard<std::mutex> lock(exceptionsAccess);
                    exceptions.push_back(std::current_exception());
                    failFlag = true;
                }

                // drop the task if failed
                if (failFlag) {
                    lock.lock();
                    jobs.pop_front();
                    lock.unlock();
                }
            }

            // if failed, go to next iteration
            if (failFlag) {
                // send a signal to threads waiting for the task to finish
                jobsCvar.notify_all();
                continue;
            }

            // wake up workers
            workersLock.lock();
            for (ThreadIndex t = 0; t < threadCount; t++)
                workers[t]->isRunning = true;
            workersLock.unlock();
            workersCvar.notify_all();    // go!

            // do the job
            if (currentJob.task)
                try {
                    do {
                        bool result = useGpuForCurrentTask ? currentJob.task->processOnGPU(*gpu, thread) : currentJob.task->process(thread);
                        if (!result) {
                            abortInternally = true;
                        }
                    } while (currentJob.mode == TaskExecutionMode::PERSISTENT && !abortInternally && !abortExternally);
                }
                catch (AnotherThreadFailed) {
                    // nothing special to do here
                }
                catch (...) {
                    eventListener.taskFail(myIndex, *currentJob.task, std::current_exception());
                    std::lock_guard<std::mutex> lock(exceptionsAccess);
                    exceptions.push_back(std::current_exception());
                    failFlag = true;
                }

            // finalizing
            workersLock.lock();

            // decrease remaining workers count
            remainingWorkers--;

            // notify other workers if they're waiting for synchro
            synchroCvar.notify_all();

            // wait until all the workers stop
            while (remainingWorkers > 0 && !thread.isTerminating)
                workersCvar.wait(workersLock);

            // call afterProcessing
            if (currentJob.task)
                try {
                    currentJob.task->afterProcessing(currentWorkerCount, useGpuForCurrentTask ? gpu : nullptr, abortExternally);
                }
                catch (...) {
                    eventListener.taskFail(myIndex, *currentJob.task, std::current_exception());
                    std::lock_guard<std::mutex> lock(exceptionsAccess);
                    exceptions.push_back(std::current_exception());
                    failFlag = true;
                }

            workersLock.unlock();

            // unlock graphic pipeline, if used
            if (useGpuForCurrentTask && gpu) {
                gpu->flush();
            }

            // call taskDone, ask if want to repeat
            bool internalRepeatFlag = false;
            if (!failFlag)
                internalRepeatFlag = eventListener.taskDone(myIndex, *currentJob.task, abortExternally);

            // drop the task
            lock.lock();
            if (!(repeatFlag || internalRepeatFlag) || failFlag) {
                jobs.pop_front();

                // send a signal to threads waiting for the task to finish
                jobsCvar.notify_all();
            }

            lock.unlock();
        }
        eventListener.threadTerminating(myIndex);

        // deleting graphic pipeline instance
        if (gpu && myIndex == 0)
            delete gpu;
    }


    /**
        Ordinary worker thread function
    */
    inline void workerThreadFunc(TaskThreadImpl& thread) {
        eventListener.threadCreated(myIndex);
        std::unique_lock<std::mutex> lock(workersAccess);
        Job myLastJob = (Job)-1;
        while (!thread.isTerminating) {
            // wait for a job
            while ((!thread.isRunning || thread.index >= currentWorkerCount || myLastJob - currentJob.id >= 0)
                    && !thread.isTerminating)
            {
                std::this_thread::yield();
                workersCvar.wait(lock);
            }
            if (thread.isTerminating) {
                lock.unlock();
                break;
            }

            // do the job
            JobContext job = currentJob;
            lock.unlock();

            /* UNLOCKED SECTION */
            try {
                do {
                    if (!job.task->process(thread)) {
                        abortInternally = true;
                    }
                } while (job.mode == TaskExecutionMode::PERSISTENT && !abortInternally && !abortExternally && !thread.isTerminating);
            }
            catch (AnotherThreadFailed) {
                // nothing special to do here
            }
            catch (...) {
                eventListener.taskFail(myIndex, *job.task, std::current_exception());
                std::lock_guard<std::mutex> lock(exceptionsAccess);
                exceptions.push_back(std::current_exception());
                failFlag = true;
            }

            // finalizing
            lock.lock();

            // decrease remaining workers count
            myLastJob = job.id;
            remainingWorkers--;

            // notify other workers if they're waiting for synchro
            synchroCvar.notify_all();

            // stop
            thread.isRunning = false;

            // send a signal to other workers
            workersCvar.notify_all();
        }

        eventListener.threadTerminating(myIndex);
    }


    inline void synchronizeThread(TaskThreadImpl& thread) {
        std::unique_lock<std::mutex> lock(synchro);    //<------- LOCKING HERE
        syncHitsCount++;

        // check if this thread is the last one passing the synchronization point
        if (syncHitsCount >= syncHitsBound + remainingWorkers) {
            syncHitsBound = syncHitsCount;
            lock.unlock();    //<------- UNLOCKING HERE
            // do not block, wake up the others
            synchroCvar.notify_all();
        }

        else {
            // Wait while other threads reach this synchronization point or the remaining number of workers drops
            // (at the end of the task), or pool is terminating.
            // Do not check if the task aborted here to keep threads synchronized.
            const int myBound = syncHitsBound;
            while (!thread.isTerminating  &&  !failFlag  &&  myBound + remainingWorkers - syncHitsCount > 0)
                synchroCvar.wait(lock);

            lock.unlock();    //<------- UNLOCKING HERE
        }

        if (failFlag)
            throw AnotherThreadFailed();
    }

    typedef struct {
        Job id;
        AbstractTask* task;
        TaskExecutionMode mode;
    } JobContext;

    TaskThreadImpl** workers;       //!< workers instances

    GraphicPipeline* gpu;           //!< THE graphic pipeline to run tasks on GPU

    std::deque<JobContext> jobs;    //!< jobs queue
    std::deque<std::exception_ptr>
        exceptions;                 //!< exceptions thrown in this pool

    JobContext currentJob;          //!< job being run at the moment
    Job jobCounter;

    ThreadIndex threadCount;        //!< actual number of workers

    ThreadIndex
        currentWorkerCount,
        remainingWorkers;           //!< number of workers performing the current task right now

    std::condition_variable
        synchroCvar,                //!< gets notified about workers synchronization
        jobsCvar,                   //!< gets notified about the jobs queue updates
        workersCvar;                //!< gets notified about workers lifecycle updates

    std::mutex
        synchro,                    //!< workers synchronization control within the current task
        workersAccess,              //!< workers lifecycle access control
        jobsAccess,                 //!< jobs queue access control
        exceptionsAccess;           //!< exceptions queue access control

    int
        syncHitsCount,              //!< number of times synchronization is hit so far by all workers together
        syncHitsBound;              //!< last sync hits count when all the workers were synchronized

    bool
        isGpuTested,                //!< if `true`, there was an attempt to warm up the GPU
        abortExternally,            //!< if `true`, the task is aborted externally
        abortInternally,            //!< if `true`, the task aborts itself: beforeProcessing(), process() or processOnGPU() returned `false`
        failFlag,                   //!< communicates to all the threads that the current task is to skip because of a problem
        repeatFlag;                 //!< if `true`, the current task is asked to be repeated

    EventListener& eventListener;

public:
    const PoolIndex myIndex;        //!< the index of the current pool

    inline ThreadPool(const PoolIndex index, const ThreadIndex limitThreadCount, EventListener & listener) :
        gpu(nullptr),
        currentJob{0, nullptr},
        jobCounter(1),
        threadCount(limitThreadCount),
        currentWorkerCount(0), remainingWorkers(0),
        syncHitsCount(0), syncHitsBound(0),
        isGpuTested(false),
        abortExternally(false), abortInternally(false),
        failFlag(false), repeatFlag(false),
        eventListener(listener),
        myIndex(index)
    {
        workers = new TaskThreadImpl*[threadCount];
        // spawning workers
        for (ThreadIndex t = 0; t < threadCount; t++)
            workers[t] = new TaskThreadImpl(t, *this);
    }


    inline ~ThreadPool() {
        // set termination flags
        {
            std::lock_guard<std::mutex> lockJobs(jobsAccess), lockWorkers(workersAccess), lockSync(synchro);
            abortExternally = true;
            for (ThreadIndex t = 0; t < threadCount; t++)
                workers[t]->isTerminating = true;
        }
        synchroCvar.notify_all();
        jobsCvar.notify_all();
        workersCvar.notify_all();
        // no wait here!
        for (ThreadIndex t = 0; t < threadCount; t++) {
            workers[t]->internalThread.join();
            delete workers[t];
        }
        delete[] workers;
    }


    /**
        Checks if the thread pool is doing great.
        If not, i.e., if there was an exception, rethrows the exception.
        There might be multiple exceptions in the queue. They are thrown one by one every time this function is called.
    */
    inline void check() {
        std::lock_guard<std::mutex> lock(exceptionsAccess);
        if (!exceptions.empty()) {
            std::exception_ptr pointer(exceptions.front());
            exceptions.pop_front();
            std::rethrow_exception(pointer);
        }
    }


    /**
        Resizes the pool.
        Blocking if there is a task running.
        \param newThreadCount       The new number of worker threads
     */
    inline void resize(ThreadIndex newThreadCount) {
        if (newThreadCount == threadCount)
            return;
        std::unique_lock<std::mutex> jobsLock(jobsAccess);
        // wait for task, if any
        while (!jobs.empty())
            jobsCvar.wait(jobsLock);

        std::unique_lock<std::mutex> workersLock(workersAccess);
        // set termination flags for threads to be stopped, if any
        for (ThreadIndex t = newThreadCount; t < threadCount; t++)
            workers[t]->isTerminating = true;

        // unlock and notify
        workersLock.unlock();
        synchroCvar.notify_all();
        jobsCvar.notify_all();
        workersCvar.notify_all();

        // join
        for (ThreadIndex t = newThreadCount; t < threadCount; t++) {
            workers[t]->internalThread.join();
            delete workers[t];
        }

        // spawn new threads if needed
        workersLock.lock();
        if (threadCount < newThreadCount) {
            TaskThreadImpl** newWorkers = new TaskThreadImpl*[newThreadCount];
            for (ThreadIndex t = 0; t < threadCount; t++)
                newWorkers[t] = workers[t];
            for (ThreadIndex t = threadCount; t < newThreadCount; t++)
                newWorkers[t] = new TaskThreadImpl(t, *this);
            delete[] workers;
            workers = newWorkers;
        }
        // update thread count
        threadCount = newThreadCount;

        workersLock.unlock();
        jobsLock.unlock();
    }


    /**
        Adds a new task to the jobs queue.
    */
    inline Job submitTask(AbstractTask& task, const TaskExecutionMode mode) {
        std::unique_lock<std::mutex> lock(jobsAccess);
        const Job job = jobCounter++;

        // add new job
        jobs.emplace_back(JobContext{job, &task, mode});

        lock.unlock();
        jobsCvar.notify_all();
        return job;
    }


    /**
        Ensures a given task executed at least once.
        \param task            The task
        \param abortCurrent    if `true` and the task is currently running, abort signal is sent.
    */
    inline Job repeatTask(AbstractTask& task, bool abortCurrent) {
        std::unique_lock<std::mutex> lock(jobsAccess);

        // check if the rask is running now, ask for repeat if it is
        if (!jobs.empty() && jobs.front().task == &task) {
            repeatFlag = true;
            if (abortCurrent)
                abortExternally = true;
            return jobs.front().id;
        }

        // check whether it is in the queue
        for (const JobContext& _ : jobs)
            if (_.task == &task) {
                return _.id;
            }

        // otherwise submit the task
        const Job job = jobCounter++;
        jobs.emplace_back(JobContext{
            job,
            &task,
            TaskExecutionMode::NORMAL
        });

        // unlock the jobs access, notify workers
        lock.unlock();
        jobsCvar.notify_all();
        return job;
    }


    /**
        Blocks until a given job finishes if not yet.
    */
    inline void waitForJob(Job job) {
        std::unique_lock<std::mutex> lock(jobsAccess);
#ifdef BEATMUP_DEBUG
        if (currentJob.task && currentJob.id != job && currentJob.mode == TaskExecutionMode::PERSISTENT && !abortExternally) {
            class BlockingOnPersistentJob : public Exception {
            public:
                BlockingOnPersistentJob(): Exception("Waiting for a persistent job to finish: potential deadlock") {}
            };
            throw BlockingOnPersistentJob();
        }
#endif
        while (true) {
            // check if the job is in the queue
            bool found = false;
            for (const JobContext& _ : jobs)
                if (_.id == job) {
                    found = true;
                    break;
                }

            // if not, done
            if (!found)
                return;

            // otherwise wait a round and check again
            jobsCvar.wait(lock);
        }
    }


    /**
        Aborts a given submitted job.
        \return `true` if the job was interrupted while running.
    */
    bool abortJob(Job job) {
        std::unique_lock<std::mutex> lock(jobsAccess);

        // check if the rask is running now, abort if it is
        if (currentJob.task && currentJob.id == job) {
            abortExternally = true;
            while (!jobs.empty() && currentJob.task && currentJob.id == job)
                jobsCvar.wait(lock);
            return true;
        }

        for (auto it = jobs.begin(); it != jobs.end(); it++)
            if (it->id == job) {
                jobs.erase(it);
                return false;
            }

        return false;
    }


    /**
        Blocks until all the submitted jobs are executed.
    */
    void wait() {
        std::unique_lock<std::mutex> lock(jobsAccess);
        while (!jobs.empty())
            jobsCvar.wait(lock);
    }


    /**
        Checks whether the pool has jobs.
    */
    inline bool busy() {
        std::lock_guard<std::mutex> lock(jobsAccess);
        return !jobs.empty();
    }


    /**
        Returns optimal number of threads depending on the hardware capabilities
    */
    inline static ThreadIndex hardwareConcurrency() {
        unsigned int N = std::thread::hardware_concurrency();
        RuntimeError::check(N > 0, "Unable to determine hardware concurrency capabilities.");
        return N > MAX_THREAD_INDEX ? MAX_THREAD_INDEX : (ThreadIndex)N;
    }


    /**
        \return `true` if GPU was queried.
    */
    inline bool isGpuQueried() const {
        return isGpuTested;
    }


    /**
        \return `true` if GPU is ready to use.
    */
    inline bool isGpuReady() const {
        return gpu != nullptr;
    }

    /**
        \returns `true` if invoked from the manager thread
    */
    inline bool isManagingThread() const {
        if (!workers)
            return false;
        return std::this_thread::get_id() == workers[0]->internalThread.get_id();
    }

    ThreadIndex getThreadCount() const { return threadCount; }
};

/**
	Executing a task in multiple threads
*/
#pragma once
#include "parallelism.h"

using namespace Beatmup;

class ThreadPool {

    class TaskThreadImpl : public TaskThread {
    private:
        inline void threadFunc() {
            current == 0 ? pool.managingThreadFunc(*this) : pool.workerThreadFunc(*this);
        }

    public:
        inline TaskThreadImpl(ThreadIndex current, ThreadPool& pool) :
                TaskThread(current),
                current(current), pool(pool), terminateFlag(false), running(false), syncPointIdx(0),
                internalThread(&TaskThreadImpl::threadFunc, this)
        {}


        /**
            \return overall number of threads working on current task.
        */
        ThreadIndex totalThreads() const {
            return pool.currentWorkerCount;
        }


        /**
            Returns `true` if the task is asked to stop from outside.
        */
        bool isTaskAborted() const {
            return pool.abortExternally;
        }


        void synchronize() {
            if (totalThreads() > 1)
                pool.synchronizeThread(*this);
        }

        ThreadPool& pool;
        ThreadIndex current;			//!< current thread index
        unsigned int syncPointIdx;		//!< synchronization point index passed by this worker
        bool running;					//!< if not, the thread will sleep
        bool terminateFlag;				//!< if `true`, the thread is requested to terminate
        // declaration order matters!
        std::thread internalThread;		//!< worker's thread
    };

public:
    /**
     * Way how to the task should be performed in the pool
     */
    enum class TaskExecutionMode {
        NORMAL,			//!< a normal task, should be run it once
        PERSISTENT		//!< a persistent task, run process(...) until it returns `false`
    };

    /**
        Event listener class
    */
    class EventListener {
    public:
        virtual inline void threadCreated(PoolIndex pool) {};				//!< called when a new worker thread is initiated
        virtual inline void threadTerminating(PoolIndex pool) {};			//!< called when a worker thread is terminating

        /*
         * Called when a task is done or cancelled; if returns `true`, the task will be repeated
         */
        virtual inline bool taskDone(PoolIndex pool, AbstractTask& task, bool aborted) {
            return false;
        }

        /*
         * Called when an exception is thrown
         */
        virtual inline void taskFail(PoolIndex pool, AbstractTask& task, const std::exception& ex) {};

        virtual inline void gpuInitFail(PoolIndex pool, const std::exception& ex) {};
    };

private:
    ThreadPool(const ThreadPool&) = delete;

    /**
        Managing (#0) worker thread function
    */
    inline void managingThreadFunc(TaskThreadImpl& thread) {
        eventListener.threadCreated(myIndex);

        // acquire this mutex now
        std::unique_lock<std::mutex> lock(mainAccess);

        // acquire these mutexes later
        std::unique_lock<std::mutex> taskDropLock(taskDropAccess, std::defer_lock);
        std::unique_lock<std::mutex> workersLock(workersAccess, std::defer_lock);

        while (!thread.terminateFlag) {

            // wait while a task is got
            while (!task && !thread.terminateFlag)
                mainCvar.wait(lock);

            if (thread.terminateFlag) {
                lock.unlock();
                break;
            }

            // init
            if (task) {
                syncHits = 0;
                syncPointCtr = 0;
                abortExternally = abortInternally = false;
                failFlag = false;
                currentWorkerCount = remainingWorkers = std::min(task->maxAllowedThreads(), threadCount);
            }

            // test execution mode
            AbstractTask::ExecutionTarget exTarget;
            TaskExecutionMode exMode = taskMode;
            bool useGpuForCurrentTask = false;
            if (task) {
                exTarget = task->getExecutionTarget();

                // only the pool "0" may use GPU!
                if (exTarget != AbstractTask::ExecutionTarget::doNotUseGPU && myIndex == 0) {

                    // test GPU if not yet
                    if (!isGpuTested) {
                        try {
                            gpu = new GraphicPipeline();
                        }
                        catch (const Exception& ex) {
                            eventListener.gpuInitFail(myIndex, ex);
                            gpu = NULL;
                        }
                        isGpuTested = true;
                    }

                    useGpuForCurrentTask = (gpu != NULL);
                    if (useGpuForCurrentTask)
                        gpu->lock();
                }

                // if GPU is required and not available, report
                if (!useGpuForCurrentTask && exTarget == AbstractTask::ExecutionTarget::useGPU) {
                    Beatmup::Exception noGpuException(
                            myIndex == 0 ?
                            "A task requires GPU, but GPU init is failed" :
                            "A task requiring GPU may only be run in the first pool"
                    );
                    eventListener.taskFail(myIndex, *task, noGpuException);
                    failFlag = true;
                }
                else
                    // run beforeProcessing
                    try {
                        task->beforeProcessing(currentWorkerCount, useGpuForCurrentTask ? gpu : NULL);
                    }
                    catch (const Exception& ex) {
                        eventListener.taskFail(myIndex, *task, ex);
                        failFlag = true;
                    }

                // drop the task if failed
                if (failFlag) {
                    taskDropLock.lock();
                    task = NULL;
                    taskDropLock.unlock();
                }
            }

            // if failed, go to next iteration
            if (failFlag) {
                // send a signal to threads waiting for the task to finish
                mainCvar.notify_all();
                if (useGpuForCurrentTask && gpu)
                    gpu->unlock();
                continue;
            }

            // setting up local stuff
            thread.syncPointIdx = 0;

            // wake up workers
            workersLock.lock();
            for (ThreadIndex t = 0; t < threadCount; t++)
                workers[t]->running = true;
            workersLock.unlock();
            workersCvar.notify_all();	// go!

            // do the job
            try {
                do {
                    bool result = useGpuForCurrentTask ? task->processOnGPU(*gpu, thread) : task->process(thread);
                    if (!result) {
                        // if stopped, set the flag and notify other workers if they're waiting for synchro
                        abortInternally = true;
                        synchroCvar.notify_all();
                    }
				} while (exMode == TaskExecutionMode::PERSISTENT && !abortInternally && !abortExternally);
            }
            catch (const Exception& ex) {
                eventListener.taskFail(myIndex, *task, ex);
                synchroCvar.notify_all();
                failFlag = true;
            }


            // finalizing
            workersLock.lock();

            // decrease remaining workers count
            remainingWorkers--;

            // call afterProcessing if no more threads are working
            if (remainingWorkers == 0) {
                try {
                    task->afterProcessing(currentWorkerCount, abortExternally);
                }
                catch (const Exception& ex) {
                    eventListener.taskFail(myIndex, *task, ex);
                    failFlag = true;
                }
            }

            // wait until all the workers stop
            while (remainingWorkers > 0)
                workersCvar.wait(workersLock);
            workersLock.unlock();

            // unlock graphic pipeline, if needed
            if (useGpuForCurrentTask && gpu)
                gpu->unlock();

            // call taskDone, ask if want to repeat
            bool internalRepeatFlag = false;
            if (!failFlag)
                internalRepeatFlag = eventListener.taskDone(myIndex, *task, abortExternally);

            // drop the task
            taskDropLock.lock();
            if (!(repeatFlag || internalRepeatFlag) || failFlag) {
                task = NULL;

                // send a signal to threads waiting for the task to finish
                mainCvar.notify_all();
            }

            repeatFlag = false;
            taskDropLock.unlock();
        }
        eventListener.threadTerminating(myIndex);

		if (gpu && myIndex == 0)
			delete gpu;
    }


    /**
        Ordinary worker thread function
    */
    inline void workerThreadFunc(TaskThreadImpl& thread) {
        eventListener.threadCreated(myIndex);
        std::unique_lock<std::mutex> lock(workersAccess);
        while (!thread.terminateFlag) {
            // wait for a job
            while (
                    !thread.running ||
                    thread.current >= currentWorkerCount
            ) {
                std::this_thread::yield();
                workersCvar.wait(lock);
                if (thread.terminateFlag)
                    break;
            }
            if (thread.terminateFlag) {
                lock.unlock();
                break;
            }

            // do the job
            TaskExecutionMode exMode = taskMode;

            lock.unlock();
            thread.syncPointIdx = 0;

            /* UNLOCKED SECTION */
            try {
                do {
                    if (!task->process(thread)) {
                        // if stopped, set the flag and notify other workers if they're waiting for synchro
                        abortInternally = true;
                        synchroCvar.notify_all();
                    }
				} while (exMode == TaskExecutionMode::PERSISTENT && !abortInternally && !abortExternally);
            }
            catch (const Exception& ex) {
                eventListener.taskFail(myIndex, *task, ex);
                synchroCvar.notify_all();
                failFlag = true;
            }

            // finalizing
            lock.lock();

            // decrease remaining workers count
            remainingWorkers--;

            // call afterProcessing if no more threads are working
            if (remainingWorkers == 0) {
                try {
                    task->afterProcessing(currentWorkerCount, abortExternally);
                }
                catch (const Exception& ex) {
                    eventListener.taskFail(myIndex, *task, ex);
                    failFlag = true;
                }
            }

            // stop
            thread.running = false;

            // send a sign to other workers
            workersCvar.notify_all();
        }

        eventListener.threadTerminating(myIndex);
    }


    inline void synchronizeThread(TaskThreadImpl& thread) {
        std::unique_lock<std::mutex> lock(synchro);
        syncHits++;
        thread.syncPointIdx++;
        synchroCvar.notify_all();

        // wait while other threads reach this syncronization point
        while (!(abortExternally || abortInternally) &&  syncHits < currentWorkerCount &&  thread.syncPointIdx > syncPointCtr)
            synchroCvar.wait(lock);

        // if we are in the first thread resuming after synchronization, step sync point counter in the executor
        if (syncHits > 0) {
            syncHits = 0;
            syncPointCtr++;
        }
    }

    TaskThreadImpl** workers;		//!< workers instances

    GraphicPipeline* gpu;			//!< THE graphic pipeline to run tasks on GPU

    AbstractTask* task;				//!< current task
    TaskExecutionMode taskMode;     //!< execution mode of the current task

    ThreadIndex threadCount;	    //!< actual number of workers

    ThreadIndex
            currentWorkerCount,
            remainingWorkers,		//!< number of workers performing the current task right now
            syncHits;				//!< number of workers reached current synchronization point

    unsigned int syncPointCtr;	//!< current synchronization point index

    std::condition_variable
            synchroCvar,			//!< conditional variable controlling workers synchronization within a task
            mainCvar,				//!< conditional variable controlling task execution
            workersCvar;			//!< conditional variable controlling global workers synchronization

    std::mutex
            synchro,				//!< mutex controlling workers synchronization
            workersAccess,			//!< access to workers
            taskDropAccess,			//!< access to task drop action (after a task is executed, it is dropped if no repeat required)
            mainAccess;				//!< main access control to the task execution state

    bool
            isGpuTested,			//!< if `true`, there was an attempt to warm up the GPU
            abortExternally,		//!< if `true`, the task is aborted externally
            abortInternally,        //!< if `true`, the task aborts itself
            failFlag,				//!< communicates to all the threads that the current task is to skip because of a problem
            runFlag,				//!< while `true`, the task will be repeated once it stops
            repeatFlag;				//!< if `true`, the current task is asked to be repeated

    EventListener& eventListener;

public:
    const PoolIndex myIndex;		//!< the index of the current pool

    inline ThreadPool(const PoolIndex index, const ThreadIndex limitThreadCount, EventListener& listener) :
            myIndex(index),
            threadCount(limitThreadCount),
            failFlag(false), abortExternally(false), abortInternally(false), repeatFlag(false), task(NULL),
            currentWorkerCount(0), remainingWorkers(0), syncHits(0), syncPointCtr(0),
            gpu(NULL), isGpuTested(false),
            eventListener(listener)
    {
        workers = new TaskThreadImpl*[threadCount];
        // spawning workers
        for (ThreadIndex t = 0; t < threadCount; t++)
            workers[t] = new TaskThreadImpl(t, *this);
    }

    /**
     * Resizes the pool.
     * Blocking if there is a task running.
     * \param newThreadCount		sets new number of workers
     */
    inline void resize(ThreadIndex newThreadCount) {
        if (newThreadCount == threadCount)
            return;
        std::unique_lock<std::mutex> lock(mainAccess);
        // wait for task, if any
        while (task)
            mainCvar.wait(lock);

        // remove unnecessary threads, if any
        for (ThreadIndex t = newThreadCount; t < threadCount; t++)
            workers[t]->terminateFlag = true;
        synchroCvar.notify_all();
        mainCvar.notify_all();
        workersCvar.notify_all();
        for (ThreadIndex t = newThreadCount; t < threadCount; t++) {
            workers[t]->internalThread.join();
            delete workers[t];
        }

        // increase size if needed
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
        lock.unlock();
    }


    inline ~ThreadPool() {
        abortExternally = true;
        for (ThreadIndex t = 0; t < threadCount; t++)
            workers[t]->terminateFlag = true;
        synchroCvar.notify_all();
        mainCvar.notify_all();
        workersCvar.notify_all();
        // no waitForTask here!
        for (ThreadIndex t = 0; t < threadCount; t++) {
            workers[t]->internalThread.join();
            delete workers[t];
        }
        delete[] workers;
    }


    /**
        Initiates new task execution. If there is a task running, blocks until it is finished.
    */
    inline void startTask(AbstractTask& task, const TaskExecutionMode mode) {
        // finish current task, if any
        std::unique_lock<std::mutex> lock(mainAccess);
        while (this->task)
            mainCvar.wait(lock);

        // set new task
        this->task = &task;
        this->taskMode = mode;
        repeatFlag = false;

        // check mode
#ifdef BEATMUP_DISABLE_GPU
        if (task.getExecutionMode() == AbstractTask::ExecutionTarget::useGPU)
            BEATMUP_ERROR("GPU is not available");
#endif

        lock.unlock();
        mainCvar.notify_all();
    }


    /**
        Initiates task repetition or new run. If there is another task running, blocks until it is finished.
        \param task				the task
        \param abortCurrent		if `true` and the same task is currently running, abort signal is sent
    */
    inline void repeatTask(AbstractTask& task, bool abortCurrent) {
        // acquiring task drop access
        std::unique_lock<std::mutex> dropLock(taskDropAccess);
        if (this->task == &task) {
            // the task is running now, just ask to repeat it
            repeatFlag = true;
            if (abortCurrent)
                abortExternally = true;
            dropLock.unlock();
        }
        else {
            // the task is not running, set it
            dropLock.unlock();
            startTask(task, taskMode);
        }
    }


    /**
        Blocks until current task is running, if any
    */
    inline void waitForTask(bool abort = false) {
        std::unique_lock<std::mutex> lock(mainAccess);
        if (!task) {
            lock.unlock();
            return;
        }
        if (abort)
            abortExternally = true;
        while (task)
            mainCvar.wait(lock);
        lock.unlock();
    }


	inline bool busy() const {
		return this->task != nullptr;
	}


    /**
        Returns optimal number of threads depending on the hardware capabilities
    */
    inline static ThreadIndex hardwareConcurrency() {
        unsigned int N = std::thread::hardware_concurrency();
        if (N <= 0)
            BEATMUP_ERROR("Unable to determine hardware concurrency capabilities (got %d).", N);
        return N > MAX_THREAD_INDEX ? MAX_THREAD_INDEX : (ThreadIndex)N;
    }


    /**
        \return `true` if GPU was queried
    */
    inline bool isGpuQueried() const {
        return isGpuTested;
    }


    /**
        \return graphic pipeline
    */
    inline GraphicPipeline* getGraphicPipeline() const {
        return gpu;
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
/**
	Executing a task in multiple threads
*/
#pragma once
#include "parallelism.h"
#include <deque>

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

        // locks
        std::unique_lock<std::mutex> lock(jobsAccess, std::defer_lock);
        std::unique_lock<std::mutex> workersLock(workersAccess, std::defer_lock);

		while (!thread.terminateFlag) {
			// wait while a task is got
			lock.lock();
			while (jobs.empty() && !thread.terminateFlag)
				mainCvar.wait(lock);

            if (thread.terminateFlag) {
                lock.unlock();
                break;
            }

            // fetch a task
            if (!jobs.empty()) {
				currentJob = jobs.front();
                syncHits = 0;
                syncPointCtr = 0;
                abortExternally = abortInternally = false;
                failFlag = false;
                currentWorkerCount = remainingWorkers = std::min(currentJob.task->maxAllowedThreads(), threadCount);
            }
			else
				currentJob.task = nullptr;

			// release queue access
			lock.unlock();

            // test execution mode
            AbstractTask::ExecutionTarget exTarget;
            bool useGpuForCurrentTask = false;
            if (currentJob.task) {
                exTarget = currentJob.task->getExecutionTarget();

                if (exTarget != AbstractTask::ExecutionTarget::doNotUseGPU && myIndex == 0) {
                    // test GPU if not yet
                    if (!isGpuTested) {
                        try {
                            gpu = new GraphicPipeline();
                        }
                        catch (const Exception& ex) {
                            eventListener.gpuInitFail(myIndex, ex);
                            gpu = nullptr;
                        }
                        isGpuTested = true;
                    }

                    useGpuForCurrentTask = (gpu != NULL);
                    if (useGpuForCurrentTask)
                        gpu->lock();
				}

                // if GPU is required and not available, report
                if (!useGpuForCurrentTask && exTarget == AbstractTask::ExecutionTarget::useGPU) {
                    Beatmup::RuntimeError noGpuException(
						myIndex == 0 ?
						"A task requires GPU, but GPU init is failed" :
						"A task requiring GPU may only be run in the main pool"
					);
                    eventListener.taskFail(myIndex, *currentJob.task, noGpuException);
                    failFlag = true;
                }
                else
                    // run beforeProcessing
                    try {
                        currentJob.task->beforeProcessing(currentWorkerCount, useGpuForCurrentTask ? gpu : nullptr);
                    }
                    catch (const Exception& ex) {
                        eventListener.taskFail(myIndex, *currentJob.task, ex);
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
			if (currentJob.task)
				try {
					do {
                	    bool result = useGpuForCurrentTask ? currentJob.task->processOnGPU(*gpu, thread) : currentJob.task->process(thread);
            	        if (!result) {
							// if stopped, set the flag and notify other workers if they're waiting for synchro
							abortInternally = true;
							synchroCvar.notify_all();
						}
					} while (currentJob.mode == TaskExecutionMode::PERSISTENT && !abortInternally && !abortExternally);
				}
				catch (const Exception& ex) {
					eventListener.taskFail(myIndex, *currentJob.task, ex);
					synchroCvar.notify_all();
					failFlag = true;
				}

            // finalizing
            workersLock.lock();

            // decrease remaining workers count
            remainingWorkers--;

            // call afterProcessing if no more threads are working
            if (currentJob.task && remainingWorkers == 0)
                try {
                    currentJob.task->afterProcessing(currentWorkerCount, abortExternally);
                }
                catch (const Exception& ex) {
                    eventListener.taskFail(myIndex, *currentJob.task, ex);
                    failFlag = true;
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
                internalRepeatFlag = eventListener.taskDone(myIndex, *currentJob.task, abortExternally);

            // drop the task
            lock.lock();
            if (!(repeatFlag || internalRepeatFlag) || failFlag) {
                jobs.pop_front();

                // send a signal to threads waiting for the task to finish
                mainCvar.notify_all();
            }

            repeatFlag = false;
            lock.unlock();
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
            JobContext job = currentJob;
            lock.unlock();
            thread.syncPointIdx = 0;

            /* UNLOCKED SECTION */
            try {
                do {
                    if (!job.task->process(thread)) {
                        // if stopped, set the flag and notify other workers if they're waiting for synchro
                        abortInternally = true;
                        synchroCvar.notify_all();
                    }
				} while (job.mode == TaskExecutionMode::PERSISTENT && !abortInternally && !abortExternally);
            }
            catch (const Exception& ex) {
                eventListener.taskFail(myIndex, *job.task, ex);
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
                    job.task->afterProcessing(currentWorkerCount, abortExternally);
                }
                catch (const Exception& ex) {
                    eventListener.taskFail(myIndex, *job.task, ex);
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

	typedef struct {
		Job id;
		AbstractTask* task;
		TaskExecutionMode mode;
	} JobContext;

    TaskThreadImpl** workers;		//!< workers instances

    GraphicPipeline* gpu;			//!< THE graphic pipeline to run tasks on GPU

    std::deque<JobContext> jobs;    //!< jobs queue
	JobContext currentJob;
	Job jobCounter;

    ThreadIndex threadCount;	    //!< actual number of workers

    ThreadIndex
            currentWorkerCount,
            remainingWorkers,		//!< number of workers performing the current task right now
            syncHits;				//!< number of workers reached current synchronization point

    unsigned int syncPointCtr;      //!< current synchronization point index

    std::condition_variable
            synchroCvar,			//!< workers synchronization control within the current task
            mainCvar,				//!< external access control to the managing thread
            workersCvar;			//!< workers lifecycle access control

    std::mutex
            synchro,				//!< workers synchronization control within the current task
            workersAccess,			//!< workers lifecycle access control
            jobsAccess;             //!< jobs queue access control

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
			jobCounter(1),
            threadCount(limitThreadCount),
            failFlag(false), abortExternally(false), abortInternally(false), repeatFlag(false),
            currentWorkerCount(0), remainingWorkers(0), syncHits(0), syncPointCtr(0),
            gpu(nullptr), isGpuTested(false),
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
        std::unique_lock<std::mutex> lock(jobsAccess);
        // wait for task, if any
        while (!jobs.empty())
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
        // no wait here!
        for (ThreadIndex t = 0; t < threadCount; t++) {
            workers[t]->internalThread.join();
            delete workers[t];
        }
        delete[] workers;
    }


    /**
        Adds a new task to the jobs queue.
    */
    inline Job submitTask(AbstractTask& task, const TaskExecutionMode mode) {
        std::unique_lock<std::mutex> lock(jobsAccess);
		const Job job = jobCounter++;

        // set new task
		jobs.emplace_back(JobContext{job, &task, mode});

        lock.unlock();
        mainCvar.notify_all();
		return job;
    }


    /**
        Ensures a given task executed at least once.
        \param task            The task
        \param abortCurrent    if `true` and the task is currently running, abort signal is sent.
    */
    inline Job repeatTask(AbstractTask& task, bool abortCurrent) {
        std::lock_guard<std::mutex> lock(jobsAccess);
		// check if the rask is running now, ask for repeat if it is
        if (!jobs.empty() && jobs.front().task == &task) {
            repeatFlag = true;
            if (abortCurrent)
                abortExternally = true;
			return jobs.front().id;
        }

		//check whether it is in the queue
		for (const JobContext& _ : jobs)
			if (_.task == &task)
				return _.id;

		// otherwise submit the task
		const Job job = jobCounter++;
		jobs.emplace_back(JobContext{
			job,
			&task,
			TaskExecutionMode::NORMAL
		});
		return job;
    }


    /**
        Blocks until a given job finishes if not yet.
    */
    inline void waitForJob(Job job) {
		std::unique_lock<std::mutex> lock(jobsAccess);
		while (true) {
			// check if the job is in the queue
			bool found = false;
			for (const JobContext& _ : jobs)
				if (_.id == job) {
					found = true;
					break;
				}
			// if not, done
			if (!found) {
				lock.unlock();
				return;
			}

			// otherwise wait a round and check again
			mainCvar.wait(lock);
		}
    }


	/**
		Aborts a given submitted job.
		\return `true` if the job was interrupted while running.
	*/
	bool abortJob(Job job) {
		std::unique_lock<std::mutex> lock(jobsAccess);

		// check if the rask is running now, abort if it is
        if (!jobs.empty() && jobs.front().id == job) {
            abortExternally = true;
			while (!jobs.empty() && jobs.front().id == job);
				mainCvar.wait(lock);
			lock.unlock();
			return true;
        }

		for (auto it = jobs.begin(); it != jobs.end(); it++)
			if (it->id == job) {
				jobs.erase(it);
				lock.unlock();
				return false;
			}

		lock.unlock();
		return false;
	}


	/**
		Blocks until all the submitted jobs are executed.
	*/
	void wait() {
		std::unique_lock<std::mutex> lock(jobsAccess);
		while (!jobs.empty())
			mainCvar.wait(lock);
		lock.unlock();
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
/*
	Beatmup environment
*/

#pragma once

#include "basic_types.h"
#include "parallelism.h"

namespace Beatmup {
	namespace GL {
		class RecycleBin;
	}
	
	typedef unsigned int memchunk;

	class AbstractBitmap;

	/**
		Basic class: task and memory management, any kind of static data
	*/
	class Environment : public Object {
		Environment(const Environment&) = delete;	//!< disabling copying constructor
	private:
		class Impl;
		Impl* impl;
		GL::RecycleBin* recycleBin;					//!< stores GPU garbage: resources managed by GPU and might be freed in the managing thread only
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
				\param task		the task
				\param aborted	if `true`, the task was aborted from outside
				\returns `true` if the task is asked to be executed again. Note that even if `false` is returned, a repetition might be asked from outside.
			*/
			virtual bool taskDone(PoolIndex pool, AbstractTask& task, bool aborted) = 0;

			/**
				Called when a task fails.
				\param task		the task
				\param ex		exception caught when the task is failed
			*/
			virtual void taskFail(PoolIndex pool, AbstractTask& task, const std::exception& ex) = 0;

			/**n
				Called when GPU intialization failed
				\param ex		exception caught
			*/
			virtual void gpuInitFail(PoolIndex pool, const std::exception& ex) = 0;
		};


		Environment();
		Environment(const PoolIndex numThreadPools, const char* swapFilePrefix, const char* swapFileSuffix);
		~Environment();

		/**
			Performs a given task.
			\param task				The task
		 	\param pool				A thread pool to run the task in
			\returns task execution time
		*/
		float performTask(AbstractTask& task, const PoolIndex pool = DEFAULT_POOL);

		/**
			Start new task or unblocking demand for repetition.
			\param task				The task
			\param abortCurrent		if `true` and the same task is currently running, the abort signal is sent.
		 	\param pool				A thread pool to run the task in
		*/
		void repeatTask(AbstractTask& task, bool abortCurrent, const PoolIndex pool = DEFAULT_POOL);

		/**
		 	Starts a new task.
		 	\param task				The task
		 	\param pool				A thread pool to run the task in
		 */
		void startTask(AbstractTask& task, const PoolIndex pool = DEFAULT_POOL);

		/**
		 	Starts a new persistent task.
			Persistent task is repeated until it decides itself to quit.
		 	\param task				The task
		 	\param pool				A thread pool to run the task in
		 */
		void startPersistentTask(AbstractTask& task, const PoolIndex pool = DEFAULT_POOL);

		/**
			Wait until the current task in a given thread pool finishes, if any.
			\param abort		If `true`, abort signal is sent to the task.
		 	\param pool			The thread pool
		*/
		void waitForTask(bool abort, const PoolIndex pool = DEFAULT_POOL);

		/**
			Queries whether a given thread pool is busy with a task.
			\param pool			The thread pool to query
			\return `true` if the thread pool is running a task, `false` otherwise
		*/
		bool busy(const PoolIndex pool = DEFAULT_POOL) const;

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
			Allocates some memory
		*/
		const memchunk allocateMemory(msize size);
		
		/**
			Acquires an allocated memory chunk, putting it in RAM.
			\param chunk	the chunk id
			\returns a pointer containing the chunk data, NULL if an invalid chunk id is passed
		*/
		const pixptr acquireMemory(memchunk chunk);

		/**
			Releases an allocated memory chunk, allowing to swap it; does not necessarily free the allocated memory.
			\param chunk			the chunk id
			\param unusedAnymore	the data in this chunk are not important and can be lost
		*/
		void releaseMemory(memchunk chunk, bool unusedAnymore);

		/**
			Frees previously allocated memory.
		*/
		void freeMemory(memchunk chunk);
		
		/**
			Performs swapping of allocated but not currently used memory on disk.
			\param howMush	required memory size in bytes to free; the environment attempts to free at least the required size
			\return actual swapped memory size
		*/
		msize swapOnDisk(msize howMuch);

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
			\internal
			\return `true` if invoked from the environment managing thread
		*/
		bool isManagingThread() const;


		/**
			\return GPU recycle bin to store GPU resources that can be freed only within a GPU-aware thread
		*/
		GL::RecycleBin* getGpuRecycleBin() const;

		/**
			\return total RAM size in bytes
		*/
		static msize getTotalRam();

		/**
			\brief Initializes GPU within a given Environment.
			GPU initialization may take some time and is done when a first task using GPU is being run. Warping up
			the GPU is useful to avoid the app get stucked for some time when it launches its first task on GPU.
		*/
		void warmUpGpu();
	};
};
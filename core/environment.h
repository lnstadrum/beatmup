/*
	Environment class
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
		/**
			An event listener (bunch of callbacks)
		*/
		class EventListener {
		public:
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
			Performs a given task
		 	\param pool				the thread pool to run the task in
			\returns task execution time
		*/
		float performTask(const PoolIndex pool, AbstractTask& task);

		/**
			Start new task or unblocking demand for repetition
		 	\param pool				the thread pool to run the task in
			\param task				the task
			\param abortCurrent		if `true` and the same task is currently running, the abort signal is sent
		*/
		void repeatTask(const PoolIndex pool, AbstractTask& task, bool abortCurrent);

		/**
		 	Starts a new task.
		 	\param pool				the thread pool to run the task in
		 	\param task				the task
		 */
		void startTask(const PoolIndex pool, AbstractTask& task);

		/**
		 	Starts a new persistent task.
			Persistent task is repeated until it decides itself to quit.
		 	\param pool				the thread pool to run the task in
		 	\param task				the task
		 */
		void startPersistentTask(const PoolIndex pool, AbstractTask& task);

		/**
			Wait until the current task in a given thread pool finishes, if any
		 	\param pool			the target thread pool
			\param abort		if `true`, abort signal is sent to the task
		*/
		void waitForTask(const PoolIndex pool, bool abort);

		bool busy(const PoolIndex pool) const;

		/**
			\returns maximum number of working thread per task
		*/
		const ThreadIndex maxAllowedWorkerCount(const PoolIndex pool) const;

		/**
			Limits maximum number of threads (workers) when performing a task
			\param maxValue		number limiting the worker count
			\returns new maximum limit
		*/
		void limitWorkerCount(const PoolIndex pool, ThreadIndex maxValue);

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
		bool isGPUQueried() const;

		/**
			\return `true` if GPU was queried and ready to use
		*/
		bool isGPUReady() const;

		/**
			\internal
			\return `true` if invoked from the environment managing thread
		*/
		bool isManagingThread() const;


		/**
			\return GPU recycle bin to store GPU resources that can be freed only within a GPU-aware thread
		*/
		GL::RecycleBin* getGPURecycleBin() const;

		/**
			\return total RAM size in bytes
		*/
		static msize getTotalRAM();
	};
};
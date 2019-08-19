#include "environment.h"
#include "parallelism.h"
#include "gpu/pipeline.h"
#include "gpu/recycle_bin.h"
#include "exception.h"
#include "bitmap/abstract_bitmap.h"
#include "thread_pool.hpp"
#include <algorithm>
#include <vector>
#include <map>
#include <mutex>

#if BEATMUP_PLATFORM_WINDOWS
	#include <windows.h>
	#undef min
	#undef max
#else
	#include <unistd.h>
	#include <sys/statfs.h>
	#include <sys/sysinfo.h>
#endif


#include "debug.h"

using namespace Beatmup;


class OutOfMemory : public Beatmup::Exception {
private:
	OutOfMemory(msize size) : Exception("Out of memory: unable to allocate neither swap %lu Kbytes", size / 1024) {}
public:
	static void raise(msize size) {
		OutOfMemory error(size);
		throw error;
	}
};


/**
	Internal memory management exception
*/
class InternalMemoryManagementError : public Beatmup::Exception {
public:
	InternalMemoryManagementError(const char * message, int chunkId) : Exception("%s\nChunk: %d", message, chunkId) {}
};


/**
	Returns available disk space in bytes
*/
msize getAvailableDiskSpace() {
#if BEATMUP_PLATFORM_WINDOWS
	ULARGE_INTEGER result;
	GetDiskFreeSpaceEx(NULL, &result, NULL, NULL);
	return (msize) result.QuadPart;
#else
	struct statfs data;
	if (fstatfs(0, &data) < 0)
		throw RuntimeError("Unable to get available disk space");
	return (msize)data.f_bsize * data.f_bfree;
#endif
}


/**
	Returns available RAM size in bytes
*/
msize getAvailableMemory() {
#if BEATMUP_PLATFORM_WINDOWS
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailPhys;
#else
	struct sysinfo info;
	sysinfo(&info);
	return info.freeram * info.mem_unit;
#endif
}


/**
	Environment class implementation (pimpl)
*/
class Environment::Impl {
private:
	/**
		Thread pool event listener
	*/
	class ThreadPoolEventListener : public ThreadPool::EventListener {
	private:
		const Environment::Impl& env;

	public:
		ThreadPoolEventListener(const Environment::Impl& env) : env(env) {}

		inline void threadCreated(PoolIndex pool) {
			if (env.eventListener)
				env.eventListener->threadCreated(pool);
		};

		inline void threadTerminating(PoolIndex pool) {
			if (env.eventListener)
				env.eventListener->threadTerminating(pool);
		};

		inline bool taskDone(PoolIndex pool, AbstractTask& task, bool aborted) {
			if (env.eventListener)
				return env.eventListener->taskDone(pool, task, aborted);
			return false;
		};

		inline void taskFail(PoolIndex pool, AbstractTask& task, const std::exception& ex) {
			if (env.eventListener)
				env.eventListener->taskFail(pool, task, ex);
		}

		inline void gpuInitFail(PoolIndex pool, const std::exception& ex) {
			if (env.eventListener)
				env.eventListener->gpuInitFail(pool, ex);
		}
	};

	enum ChunkSwappingState {				//!< indicates swapping state of a chunk
		AVAILABLE = 0,
		ON_DISK,
		SOMEWHERE							//!< the chunk is okay, but it is not in main memory now (e.g. on GPU); the memory can be recycled, the data does not matter
	};

	enum SwappingOperation { SWAP, UNSWAP, CLEAR };

	typedef struct {
		msize size;							//!< the size of the chunk
		unsigned int lockCounter;			//!< number of active chunk users (the chunk may not be swapped if it is positive)
		pixptr data;						//!< chunk data, if null, the chunk is swapped
		bool toFree;						//!< must be freed when number of users is zero
		ChunkSwappingState swapping;		//!< indicates if the chunk is swapped and where it is
	} ChunkState;

	ThreadIndex optimalThreadCount;			//!< optimal default number of worker threads per task in each pool

	std::map<memchunk, ChunkState> chunks;		//!< map of all allocated memory blocks
	std::mutex memAccess;         				//!< memory access control
	std::vector<memchunk> swappableChunks;		//!< list of chunk ids in its releasing order (for swapping)
	memchunk chunkCounter;						//!< allocated memory chunk counter

	ThreadPool** threadPools;					//!< thread pools of task workers
	PoolIndex numThreadPools;
	ThreadPoolEventListener threadPoolEventListener;

	const bool swapEnabled;
	std::string
		swapFilePrefix,							//!< swap file name prefix
		swapFileSuffix;							//!< swap file name suffix

	msize memToKeepFree;						//!< RAM memory size (in bytes) to keep free (use swapping if need more)


	/**
		Swaps or unswaps a chunk from/to disk; must be called from a section having exclusive acces to chunks
		\param chunk		the chunk id to process
		\param operation	what to do
	*/
	void swapChunk(memchunk chunk, SwappingOperation operation) {
		ChunkState& C = chunks[chunk];

		// getting file name
		const size_t SIZE = swapFilePrefix.size() + swapFileSuffix.size() + 15;
		char * fileName = new char[SIZE];
#ifdef _MSC_VER
		sprintf_s(fileName, SIZE, "%s%04d%s", swapFilePrefix.c_str(), chunk, swapFileSuffix.c_str());
#else
		snprintf(fileName, SIZE, "%s%04d%s", swapFilePrefix.c_str(), chunk, swapFileSuffix.c_str());
#endif

		if (operation == SwappingOperation::CLEAR)
			std::remove(fileName);
		else {
#ifdef BEATMUP_DEBUG
			if (C.lockCounter != 0)
				throw InternalMemoryManagementError("Trying to (un)swap a locked chunk", chunk);
			if (C.toFree)
				throw InternalMemoryManagementError("Trying to (un)swap a chunk which is to free", chunk);
#endif

			// opening file
			FILE* file;
#ifdef _MSC_VER
			fopen_s(&file, fileName, operation == SwappingOperation::SWAP ? "wb" : "rb");
#else
			file = fopen(fileName, operation == SwappingOperation::SWAP ? "wb" : "rb");
#endif
			if (!file)
				throw IOError(fileName, operation == SwappingOperation::SWAP ? "Cannot swap into" : "Cannot swap from");

			// swapping data on disk
			if (operation == SwappingOperation::SWAP) {
#ifdef BEATMUP_DEBUG
				if (C.swapping != ChunkSwappingState::AVAILABLE)
					throw InternalMemoryManagementError("Trying to swap a chunk in a wrong state", chunk);
				BEATMUP_DEBUG_I("Swapping %u Kbytes to %s", C.size / 1024, fileName);
#endif
				fwrite(C.data, C.size, 1, file);
				free(C.data);
				C.data = NULL;
				C.swapping = ChunkSwappingState::ON_DISK;
			}
			// unswapping data from disk
			else {
#ifdef BEATMUP_DEBUG
				if (C.swapping != ChunkSwappingState::ON_DISK)
					throw InternalMemoryManagementError("Trying to unswap a chunk in a wrong state", chunk);
				BEATMUP_DEBUG_I("Unswapping %u Kbytes from %s", C.size / 1024, fileName);
#endif
				C.data = allocateWithSwapping(C.size);
				fread(C.data, C.size, 1, file);
				//FIXME: C.size might be less than the actual size if a reallocation occurs
			}
			fclose(file);
			if (operation == SwappingOperation::UNSWAP)
				std::remove(fileName);
		}
		delete[] fileName;
	}


	inline void removeChunkFromSwappables(memchunk chunk) {
		auto it = std::find(swappableChunks.begin(), swappableChunks.end(), chunk);
		if (it != swappableChunks.end())
			swappableChunks.erase(it);
	}


protected:
	/**
		Performs swapping of allocated but not currently used memory on disk.
		\param howMuch	required memory size in bytes to free; the environment attempts to free at least the required size
		\return actual swapped memory size
	*/
	inline msize doSwapping(msize howMuch) {
		msize
			freed = 0,
			available = getAvailableDiskSpace();
		for (std::vector<memchunk>::iterator it = swappableChunks.begin(); it < swappableChunks.end(); ) {
			ChunkState& C = chunks[*it];
			if (C.size < available) {
				swapChunk(*it, SwappingOperation::SWAP);
				freed += C.size;
				available = getAvailableDiskSpace();
				swappableChunks.erase(it);
				if (freed >= howMuch)
					break;
			}
			else {
				it++;
			}
		}
		return freed;
	}


	inline pixptr allocateWithSwapping(msize howMuch) {
		BEATMUP_DEBUG_I("Allocating %u Kbytes (%u MB free)...", howMuch / 1024, getAvailableMemory() / 1048576);

		while (true) {
			msize avail = getAvailableMemory();
			if (avail >= howMuch + memToKeepFree)
				return (pixptr)malloc(howMuch);
			if (!swapEnabled || doSwapping(howMuch) == 0)
				OutOfMemory::raise(howMuch);
		}
	}


	/**
		Frees chunk memory
	*/
	inline void doFreeMemory(memchunk chunk) {
#ifdef BEATMUP_DEBUG
		if (chunks.count(chunk) == 0)
			throw InternalMemoryManagementError("Freeing a bad chunk", chunk);
#endif
		ChunkState& C = chunks[chunk];
		C.toFree = true;
		if (C.lockCounter == 0) {
			// freeing the chunk
			if (C.swapping == ChunkSwappingState::AVAILABLE)
				free(C.data);
			else if (C.swapping == ChunkSwappingState::ON_DISK)
				swapChunk(chunk, SwappingOperation::CLEAR);
			else if (C.swapping != ChunkSwappingState::SOMEWHERE)
				throw RuntimeError("Unimplemented memory disposing operation for specified swapping state");
			removeChunkFromSwappables(chunk);
			chunks.erase(chunk);
		}
		else
			C.lockCounter--;
	}


public:
	Environment::EventListener* eventListener;	//!< an event listener

	Impl(const PoolIndex numThreadPools, const char* swapFilePrefix, const char* swapFileSuffix) :
		optimalThreadCount(std::max<ThreadIndex>(1, ThreadPool::hardwareConcurrency() / numThreadPools)),
		numThreadPools(numThreadPools),
		chunkCounter(1),
		eventListener(nullptr),
		threadPoolEventListener(*this),
		memToKeepFree(0),
		swapEnabled(swapFilePrefix && swapFileSuffix),
		swapFilePrefix(swapFilePrefix ? swapFilePrefix : ""), swapFileSuffix(swapFileSuffix ? swapFileSuffix : "")
	{
		threadPools = new ThreadPool*[numThreadPools];
		for (PoolIndex pool = 0; pool < numThreadPools; pool++)
			threadPools[pool] = new ThreadPool(pool, optimalThreadCount, threadPoolEventListener);
	}


	~Impl() {
		for (PoolIndex i = 0; i < numThreadPools; i++)
			delete threadPools[i];
		delete[] threadPools;

		// freeing allocated memory chunks
		for (auto chunk : chunks)
			if (chunk.second.data)
				delete chunk.second.data;
	}


	float performTask(PoolIndex pool, AbstractTask& task) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		auto startTime = std::chrono::high_resolution_clock::now();
		Job job = threadPools[pool]->submitTask(task, ThreadPool::TaskExecutionMode::NORMAL);
		threadPools[pool]->waitForJob(job);
		auto endTime = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<float, std::milli>(endTime - startTime).count();
	}


	void repeatTask(PoolIndex pool, AbstractTask& task, bool abortCurrent) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		threadPools[pool]->repeatTask(task, abortCurrent);
	}


	Job submitTask(const PoolIndex pool, AbstractTask& task) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		return threadPools[pool]->submitTask(task, ThreadPool::TaskExecutionMode::NORMAL);
	}


	Job submitPersistentTask(const PoolIndex pool, AbstractTask& task) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		return threadPools[pool]->submitTask(task, ThreadPool::TaskExecutionMode::PERSISTENT);
	}


	void waitForJob(const PoolIndex pool, Job job) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		threadPools[pool]->waitForJob(job);
	}


	bool abortJob(const PoolIndex pool, Job job) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		return threadPools[pool]->abortJob(job);
	}


	void wait(const PoolIndex pool) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		threadPools[pool]->wait();
	}


	bool busy(const PoolIndex pool) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		return threadPools[pool]->busy();
	}


	const ThreadIndex maxAllowedWorkerCount(PoolIndex pool) const {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		return threadPools[pool]->getThreadCount();
	}


	void limitWorkerCount(PoolIndex pool, ThreadIndex maxValue) {
		BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
		threadPools[pool]->resize(maxValue);
	}


	const memchunk allocateMemory(msize size) {
		std::lock_guard<std::mutex> lock(memAccess);
		chunks[chunkCounter] = { size, 0, nullptr, false };
		chunks[chunkCounter].data = allocateWithSwapping(size);
		return chunkCounter++;
	}


	const pixptr acquireMemory(memchunk chunk) {
		std::lock_guard<std::mutex> lock(memAccess);
		if (!chunks.count(chunk))
			return nullptr;
		// unswapping
		ChunkState& C = chunks[chunk];
		if (C.swapping == ChunkSwappingState::ON_DISK) {
			C.data = (pixptr)allocateWithSwapping(C.size);
			swapChunk(chunk, SwappingOperation::UNSWAP);
			C.swapping = ChunkSwappingState::AVAILABLE;
		}
		else if (C.swapping == ChunkSwappingState::SOMEWHERE) {
			C.data = (pixptr)allocateWithSwapping(C.size);
			C.swapping = ChunkSwappingState::AVAILABLE;
		}
		else if (C.swapping != ChunkSwappingState::AVAILABLE)
			throw RuntimeError("Unimplemented memory reallocating operation for specified swapping state");
		C.lockCounter++;
		removeChunkFromSwappables(chunk);
		return C.data;
	}


	void releaseMemory(memchunk chunk, bool unusedAnymore) {
		std::lock_guard<std::mutex> lock(memAccess);
		if (chunks.count(chunk) > 0) {
			ChunkState& C = chunks[chunk];
			if (C.lockCounter > 0)
				C.lockCounter--;

			// if completely released
			if (C.lockCounter == 0) {
				if (C.toFree)
					doFreeMemory(chunk);
				else {
					removeChunkFromSwappables(chunk);

					if (unusedAnymore) {
						C.swapping = ChunkSwappingState::SOMEWHERE;
						free(C.data);
						C.data = NULL;
					} else
						swappableChunks.push_back(chunk);

				}
			}
		}
#ifdef BEATMUP_DEBUG
		else throw InternalMemoryManagementError("Releasing a bad chunk", chunk);
#endif
	}


	void freeMemory(memchunk chunk) {
		std::lock_guard<std::mutex> lock(memAccess);
		doFreeMemory(chunk);
	}


	msize swapOnDisk(msize howMuch) {
		std::lock_guard<std::mutex> lock(memAccess);
		return doSwapping(howMuch);
	}


	inline bool isGpuQueried() const {
		return threadPools[0]->isGpuQueried();
	}


	inline bool isGpuReady() const {
		return threadPools[0]->getGraphicPipeline() != NULL;
	}


	inline bool isManagingThread() const {
		for (PoolIndex pool = 0; pool < numThreadPools; ++pool)
			if (threadPools[pool]->isManagingThread())
				return true;
		return false;
	}
};


Environment::Environment() : Environment(1, "swap", ".tmp") {}
Environment::Environment(const PoolIndex numThreadPools, const char* swapFilePrefix, const char* swapFileSuffix) {
	impl = new Impl(numThreadPools, swapFilePrefix, swapFileSuffix);
	recycleBin = new GL::RecycleBin(*this);
}


Environment::~Environment() {
	delete impl;
	delete recycleBin;
}

float Environment::performTask(AbstractTask& task, const PoolIndex pool) {
	return impl->performTask(pool, task);
}

void Environment::repeatTask(AbstractTask& task, bool abortCurrent, const PoolIndex pool) {
	return impl->repeatTask(pool, task, abortCurrent);
}

Job Environment::submitTask(AbstractTask& task, const PoolIndex pool) {
	return impl->submitTask(pool, task);
}

Job Environment::submitPersistentTask(AbstractTask& task, const PoolIndex pool) {
	return impl->submitPersistentTask(pool, task);
}

void Environment::waitForJob(Job job, const PoolIndex pool) {
	impl->waitForJob(pool, job);
}

bool Environment::abortJob(Job job, const PoolIndex pool) {
	return impl->abortJob(pool, job);
}

void Environment::wait(const PoolIndex pool) {
	impl->wait(pool);
}

bool Environment::busy(const PoolIndex pool) {
	return impl->busy(pool);
}

const ThreadIndex Environment::maxAllowedWorkerCount(const PoolIndex pool) const {
	return impl->maxAllowedWorkerCount(pool);
}

void Environment::limitWorkerCount(ThreadIndex maxValue, const PoolIndex pool) {
	impl->limitWorkerCount(pool, maxValue);
}

const memchunk Environment::allocateMemory(msize size) {
	return impl->allocateMemory(size);
}

const pixptr Environment::acquireMemory(memchunk chunk) {
	return impl->acquireMemory(chunk);
}

void Environment::releaseMemory(memchunk chunk, bool unusedAnymore) {
	impl->releaseMemory(chunk, unusedAnymore);
}

void Environment::freeMemory(memchunk chunk) {
	impl->freeMemory(chunk);
}

msize Environment::swapOnDisk(msize howMuch) {
	return impl->swapOnDisk(howMuch);
}

void Environment::setEventListener(EventListener* eventListener) {
	impl->eventListener = eventListener;
}

Environment::EventListener* Environment::getEventListener() const {
	return impl->eventListener;
}

bool Environment::isGpuQueried() const {
	return impl->isGpuQueried();
}

bool Environment::isGpuReady() const {
	return impl->isGpuReady();
}

bool Environment::isManagingThread() const {
	return impl->isManagingThread();
}


GL::RecycleBin* Environment::getGpuRecycleBin() const {
	return recycleBin;
}


msize Environment::getTotalRam() {
#if BEATMUP_PLATFORM_WINDOWS
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPhys;
#else
	struct sysinfo info;
	sysinfo(&info);
	return info.totalram * info.mem_unit;
#endif
}


void Environment::warmUpGpu() {
	if (!isGpuReady()) {
		GpuTask task;
		performTask(task);
	}
}

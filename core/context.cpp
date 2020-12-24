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

#include "context.h"
#include "parallelism.h"
#include "gpu/pipeline.h"
#include "gpu/recycle_bin.h"
#include "gpu/gpu_task.h"
#include "exception.h"
#include "bitmap/abstract_bitmap.h"
#include "thread_pool.hpp"
#include <algorithm>
#include <vector>
#include <map>
#include <mutex>
#include <iostream>

#ifdef BEATMUP_PLATFORM_ANDROID
#include <android/log.h>
#endif

using namespace Beatmup;


/**
    Context class implementation (pimpl)
*/
class Context::Impl {
private:
    /**
        Thread pool event listener
    */
    class ThreadPoolEventListener : public ThreadPool::EventListener {
    private:
        const Context::Impl& ctx;

    public:
        ThreadPoolEventListener(const Context::Impl& ctx) : ctx(ctx) {}

        inline void threadCreated(PoolIndex pool) {
            if (ctx.eventListener)
                ctx.eventListener->threadCreated(pool);
        };

        inline void threadTerminating(PoolIndex pool) {
            if (ctx.eventListener)
                ctx.eventListener->threadTerminating(pool);
        };

        inline bool taskDone(PoolIndex pool, AbstractTask& task, bool aborted) {
            if (ctx.eventListener)
                return ctx.eventListener->taskDone(pool, task, aborted);
            return false;
        };

        inline void taskFail(PoolIndex pool, AbstractTask& task, std::exception_ptr exPtr) {
            if (ctx.eventListener)
                try {
                    std::rethrow_exception(exPtr);
                }
                catch (const std::exception & ex) {
#if BEATMUP_PLATFORM_ANDROID
                    __android_log_print(ANDROID_LOG_ERROR, "Beatmup",
                        "\n********************************************\n"
                        "Beatmup engine raises exception:\n%s"
                        "\n********************************************\n", ex.what());
#elif BEATMUP_DEBUG
                    std::cout << ex.what() << std::endl;
#endif
                    ctx.eventListener->taskFail(pool, task, ex);
                }
        }

        inline void gpuInitFail(PoolIndex pool, std::exception_ptr exPtr) {
            if (ctx.eventListener)
                try {
                    std::rethrow_exception(exPtr);
                }
                catch (const std::exception & ex) {
#if BEATMUP_PLATFORM_ANDROID
                    __android_log_print(ANDROID_LOG_ERROR, "Beatmup",
                        "\n********************************************\n"
                        "Beatmup engine was unable to init GPU:\n%s"
                        "\n********************************************\n", ex.what());
#elif BEATMUP_DEBUG
                    std::cout << ex.what() << std::endl;
#endif
                    ctx.eventListener->gpuInitFail(pool, ex);
                }
        }
    };

    ThreadIndex optimalThreadCount;			//!< optimal default number of worker threads per task in each pool

    ThreadPool** threadPools;					//!< thread pools of task workers
    PoolIndex numThreadPools;
    ThreadPoolEventListener threadPoolEventListener;


public:
    Context::EventListener* eventListener;	//!< an event listener

    Impl(const PoolIndex numThreadPools) :
        optimalThreadCount(std::max<ThreadIndex>(1, ThreadPool::hardwareConcurrency() / numThreadPools)),
        numThreadPools(numThreadPools),
        threadPoolEventListener(*this),
        eventListener(nullptr)
    {
        threadPools = new ThreadPool*[numThreadPools];
        for (PoolIndex pool = 0; pool < numThreadPools; pool++)
            threadPools[pool] = new ThreadPool(pool, optimalThreadCount, threadPoolEventListener);
    }


    ~Impl() {
        for (PoolIndex i = 0; i < numThreadPools; i++)
            delete threadPools[i];
        delete[] threadPools;
    }


    float performTask(PoolIndex pool, AbstractTask& task) {
        BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
        auto startTime = std::chrono::high_resolution_clock::now();
        Job job = threadPools[pool]->submitTask(task, ThreadPool::TaskExecutionMode::NORMAL);
        threadPools[pool]->waitForJob(job);
        auto endTime = std::chrono::high_resolution_clock::now();
        threadPools[pool]->check();
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


    void check(const PoolIndex pool) {
        threadPools[pool]->check();
    }


    const ThreadIndex maxAllowedWorkerCount(const PoolIndex pool) const {
        BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
        return threadPools[pool]->getThreadCount();
    }


    void limitWorkerCount(const PoolIndex pool, ThreadIndex maxValue) {
        BEATMUP_ASSERT_DEBUG(pool < numThreadPools);
        threadPools[pool]->resize(maxValue);
    }


    inline bool isGpuQueried() const {
        return threadPools[0]->isGpuQueried();
    }


    inline bool isGpuReady() const {
        return threadPools[0]->isGpuReady();
    }


    inline bool isManagingThread() const {
        for (PoolIndex pool = 0; pool < numThreadPools; ++pool)
            if (threadPools[pool]->isManagingThread())
                return true;
        return false;
    }
};


Context::Context() : Context(1) {}
Context::Context(const PoolIndex numThreadPools) {
    impl = new Impl(numThreadPools);
    recycleBin = new GL::RecycleBin(*this);
}


Context::~Context() {
    if (recycleBin)
        recycleBin->emptyBin();
    delete recycleBin;
    delete impl;
}

float Context::performTask(AbstractTask& task, const PoolIndex pool) {
    return impl->performTask(pool, task);
}

void Context::repeatTask(AbstractTask& task, bool abortCurrent, const PoolIndex pool) {
    return impl->repeatTask(pool, task, abortCurrent);
}

Job Context::submitTask(AbstractTask& task, const PoolIndex pool) {
    return impl->submitTask(pool, task);
}

Job Context::submitPersistentTask(AbstractTask& task, const PoolIndex pool) {
    return impl->submitPersistentTask(pool, task);
}

void Context::waitForJob(Job job, const PoolIndex pool) {
    impl->waitForJob(pool, job);
}

bool Context::abortJob(Job job, const PoolIndex pool) {
    return impl->abortJob(pool, job);
}

void Context::wait(const PoolIndex pool) {
    impl->wait(pool);
}

bool Context::busy(const PoolIndex pool) {
    return impl->busy(pool);
}

void Context::check(const PoolIndex pool) {
    impl->check(pool);
}

const ThreadIndex Context::maxAllowedWorkerCount(const PoolIndex pool) const {
    return impl->maxAllowedWorkerCount(pool);
}

void Context::limitWorkerCount(ThreadIndex maxValue, const PoolIndex pool) {
    impl->limitWorkerCount(pool, maxValue);
}

void Context::setEventListener(EventListener* eventListener) {
    impl->eventListener = eventListener;
}

Context::EventListener* Context::getEventListener() const {
    return impl->eventListener;
}

bool Context::isGpuQueried() const {
    return impl->isGpuQueried();
}

bool Context::isGpuReady() const {
    return impl->isGpuReady();
}

void Context::warmUpGpu() {
    if (!isGpuReady()) {
        GpuTask task;
        performTask(task);
    }
}

bool Context::queryGpuInfo(std::string& vendor, std::string& renderer) {
    class GpuQueryingTask : public AbstractTask {
    public:
        bool gpuFound;
        std::string &vendor, &renderer;

        GpuQueryingTask(std::string& vendor, std::string& renderer) :
            vendor(vendor), renderer(renderer) {}

        TaskDeviceRequirement getUsedDevices() const { return TaskDeviceRequirement::GPU_OR_CPU; }

        bool process(TaskThread& thread) {
            gpuFound = false;
            return true;
        }

        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
            vendor = gpu.getGpuVendorString();
            renderer = gpu.getGpuRendererString();
            gpuFound = true;
            return true;
        }
    } task(vendor, renderer);

    performTask(task);

    return task.gpuFound;
}

bool Context::isManagingThread() const {
    return impl->isManagingThread();
}

GL::RecycleBin* Context::getGpuRecycleBin() const {
    return recycleBin;
}

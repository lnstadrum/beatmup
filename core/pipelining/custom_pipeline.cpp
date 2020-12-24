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

#include "../exception.h"
#include "custom_pipeline.h"
#include <algorithm>
#include <chrono>

using namespace Beatmup;

class CustomPipeline::Impl : public TaskRouter {
private:
    std::vector<TaskHolder*>::iterator currentTask;
    std::vector<TaskHolder*> tasks;	    	//!< the list of tasks
    std::mutex tasksAccess;					//!< task list access control
    GraphicPipeline* gpu;
    TaskThread* thread;
    AbstractTask::TaskDeviceRequirement executionMode;
    ThreadIndex maxThreadCount;
    bool measured;                          //!< if `true`, the execution mode and the thread count are determined
    bool abort;                             //!< if `true`, one of threads executing the current task caused its aborting

public:
    Impl():
        measured(false)
    {}

    virtual ~Impl() {
        // destroying taskholders
        for (auto task : tasks)
            delete task;
    }

    TaskHolder& getCurrentTask() {
        return **currentTask;
    }

    const TaskHolder& getCurrentTask() const {
        return **currentTask;
    }

    void runTask() {
        auto startTime = std::chrono::high_resolution_clock::now();
        TaskHolder& task = **currentTask;
        task.getTask().beforeProcessing(
            task.threadCount,
            task.executionMode != AbstractTask::TaskDeviceRequirement::CPU_ONLY && gpu ? ProcessingTarget::GPU : ProcessingTarget::CPU,
            gpu
        );

        // wait for other workers
        thread->synchronize();

        // perform the task
        bool result;
        if (task.executionMode != AbstractTask::TaskDeviceRequirement::CPU_ONLY && gpu)
            result = task.getTask().processOnGPU(*gpu, *thread);
        else
            result = task.getTask().process(*thread);

        if (!result)
            abort = true;

        // wait for other workers
        thread->synchronize();

        task.getTask().afterProcessing(
            task.threadCount,
            task.executionMode != AbstractTask::TaskDeviceRequirement::CPU_ONLY && gpu ? gpu : nullptr,
            !abort
        );

        auto endTime = std::chrono::high_resolution_clock::now();
        task.time = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    }

    void goToNextTask() {
        currentTask++;
    }

    bool allTasksDone() const {
        return currentTask >= tasks.end();
    }

    bool allTasksAborted() const {
        return abort && thread->isTaskAborted();
    }


    int getTaskCount() {
        std::lock_guard<std::mutex> lock(tasksAccess);
        return (int) tasks.size();
    }

    TaskHolder* getTask(int index) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        BEATMUP_ASSERT_DEBUG(0 <= index && index < tasks.size());
        return tasks[index];
    }

    int getTaskIndex(const TaskHolder* holder) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        const auto& it = std::find(tasks.cbegin(), tasks.cend(), holder);
        if (it == tasks.cend())
            return -1;
        return (int) (it - tasks.cbegin());
    }

    void addTask(TaskHolder* taskHolder) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        measured = false;
        tasks.push_back(taskHolder);
    }

    void insertTask(TaskHolder* newbie, const TaskHolder* before) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        const auto& nextHolder = std::find(tasks.cbegin(), tasks.cend(), before);
        if (nextHolder == tasks.cend())
            throw RuntimeError("Reference task holder is not found in the task list");
        measured = false;
        tasks.insert(nextHolder , newbie);
    }

    bool removeTask(const TaskHolder* target) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        const auto& pointer = std::find(tasks.cbegin(), tasks.cend(), target);
        if (pointer == tasks.cend())
            return false;
        delete *pointer;
        tasks.erase(pointer);
        return true;
    }

    /**
     * Determining execution mode (GPU or CPU) and thread count for each task
     */
    void measure() {
        std::lock_guard<std::mutex> lock(tasksAccess);
        executionMode = TaskDeviceRequirement::CPU_ONLY;
        maxThreadCount = 0;
        for (auto& it : tasks) {
            switch (it->executionMode = it->getTask().getUsedDevices()) {
                case TaskDeviceRequirement::GPU_ONLY:
                    executionMode = TaskDeviceRequirement::GPU_ONLY;
                    break;
                case TaskDeviceRequirement::GPU_OR_CPU:
                    if (executionMode == TaskDeviceRequirement::CPU_ONLY)
                        executionMode = TaskDeviceRequirement::GPU_OR_CPU;
                    break;
                default:
                    break;
            }
            it->threadCount = it->getTask().getMaxThreads();
            maxThreadCount = std::max(maxThreadCount, it->threadCount);
        }
        measured = true;
    }

    AbstractTask::TaskDeviceRequirement getUsedDevices() const {
        if (!measured)
            throw PipelineNotReady("Pipeline not measured; call measure() first.");
        return executionMode;
    }

    ThreadIndex getMaxThreads() const {
        if (!measured)
            throw PipelineNotReady("Pipeline not measured; call measure() first.");
        return maxThreadCount;
    }

    void beforeProcessing(GraphicPipeline *gpu) {
        this->gpu = gpu;
        abort = false;
    }

    /**
     * Processing entry point
     */
    bool process(GraphicPipeline* gpu, TaskThread& thread, CustomPipeline & pipeline) {
        // managing worker thread
        if (thread.isManaging()) {
            this->thread = &thread;
            std::lock_guard<std::mutex> lock(tasksAccess);
            currentTask = tasks.begin();
            pipeline.route(*this);
        }

        // secondary worker thread
        else {
            do {
                thread.synchronize();
                if (!allTasksDone() && !allTasksAborted() && thread.currentThread() < (*currentTask)->threadCount)
                    if (!(*currentTask)->getTask().process(thread))
                        abort = true;
                thread.synchronize();
            } while (!allTasksDone() && !allTasksAborted());
        }

        return !abort;
    }
};


AbstractTask::TaskDeviceRequirement CustomPipeline::getUsedDevices() const {
    return impl->getUsedDevices();
}

ThreadIndex CustomPipeline::getMaxThreads() const {
    return impl->getMaxThreads();
}

void CustomPipeline::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline *gpu) {
    impl->beforeProcessing(gpu);
}

void CustomPipeline::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    AbstractTask::afterProcessing(threadCount, gpu, aborted);
}

bool CustomPipeline::process(TaskThread &thread) {
    return impl->process(nullptr, thread, *this);
}

bool CustomPipeline::processOnGPU(GraphicPipeline &gpu, TaskThread &thread) {
    return impl->process(&gpu, thread, *this);
}

CustomPipeline::CustomPipeline() :
    impl(new Impl())
{}

CustomPipeline::~CustomPipeline() {
    delete impl;
}

int CustomPipeline::getTaskCount() const {
    return impl->getTaskCount();
}

CustomPipeline::TaskHolder& CustomPipeline::getTask(int index) const {
    return *impl->getTask(index);
}

int CustomPipeline::getTaskIndex(const TaskHolder &task) {
    return impl->getTaskIndex(&task);
}

CustomPipeline::TaskHolder& CustomPipeline::addTask(AbstractTask &task) {
    TaskHolder* holder = createTaskHolder(task);
    impl->addTask(holder);
    return *holder;
}

CustomPipeline::TaskHolder& CustomPipeline::insertTask(AbstractTask &task, const TaskHolder& before) {
    TaskHolder* holder = createTaskHolder(task);
    impl->insertTask(holder, &before);
    return *holder;
}

bool CustomPipeline::removeTask(const TaskHolder& task) {
    return impl->removeTask(&task);
}

void CustomPipeline::measure() {
    impl->measure();
}

CustomPipeline::TaskHolder::TaskHolder(CustomPipeline::TaskHolder &&holder):
    task(holder.task),
    executionMode(holder.executionMode),
    threadCount(holder.threadCount),
    time(0)
{}


bool Beatmup::operator == (const CustomPipeline::TaskHolder& left, const CustomPipeline::TaskHolder& right) {
    return &left == &right;
}

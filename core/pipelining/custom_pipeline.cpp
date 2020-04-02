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
    AbstractTask::ExecutionTarget executionMode;
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

    const TaskHolder& getCurrentTask() const {
        return **currentTask;
    }

    void runTask() {
        auto startTime = std::chrono::high_resolution_clock::now();
        TaskHolder& task = **currentTask;
        task.getTask().beforeProcessing(
                task.threadCount,
                task.executionMode != AbstractTask::ExecutionTarget::doNotUseGPU && gpu ?
                    gpu : nullptr
        );

        // wait for other workers
        thread->synchronize();

        // perform the task
        bool result;
        if (task.executionMode != AbstractTask::ExecutionTarget::doNotUseGPU && gpu)
            result = task.getTask().processOnGPU(*gpu, *thread);
        else
            result = task.getTask().process(*thread);

        if (!result)
            abort = true;

        // wait for other workers
        thread->synchronize();

        task.getTask().afterProcessing(task.threadCount, !abort);

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

    void insertTask(TaskHolder* newbie, const TaskHolder* succeedingHoder) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        const auto& nextHolder = std::find(tasks.cbegin(), tasks.cend(), succeedingHoder);
        if (nextHolder == tasks.cend())
            throw RuntimeError("Reference task holder is not found in the task list");
        measured = false;
        tasks.insert(nextHolder - 1, newbie);
    }

    bool removeTask(const TaskHolder* target) {
        std::lock_guard<std::mutex> lock(tasksAccess);
        const auto& pointer = std::find(tasks.cbegin(), tasks.cend(), target);
        if (pointer == tasks.cend())
            return false;
        tasks.erase(pointer);
        delete *pointer;
        return true;
    }

    /**
     * Determining execution mode (GPU or CPU) and thread count for each task
     */
    void measure() {
        std::lock_guard<std::mutex> lock(tasksAccess);
        executionMode = ExecutionTarget::doNotUseGPU;
        maxThreadCount = 0;
        for (auto& it : tasks) {
            switch (it->executionMode = it->getTask().getExecutionTarget()) {
                case ExecutionTarget::useGPU:
                    executionMode = ExecutionTarget::useGPU;
                    break;
                case ExecutionTarget::useGPUIfAvailable:
                    if (executionMode == ExecutionTarget::doNotUseGPU)
                        executionMode = ExecutionTarget::useGPUIfAvailable;
                    break;
                default:
                    break;
            }
            it->threadCount = it->getTask().maxAllowedThreads();
            maxThreadCount = std::max(maxThreadCount, it->threadCount);
        }
        measured = true;
    }

    AbstractTask::ExecutionTarget getExecutionTarget() const {
        if (!measured)
            throw PipelineNotReady("Pipeline not measured; call measure() first.");
        return executionMode;
    }

    ThreadIndex maxAllowedThreads() const {
        if (!measured)
            throw PipelineNotReady("Pipeline not measured; call measure() first.");
        return maxThreadCount;
    }

    void beforeProcessing() {
        abort = false;
    }

    /**
     * Processing entry point
     */
    bool process(GraphicPipeline* gpu, TaskThread& thread, CustomPipeline & pipeline) {
        // managing worker thread
        if (thread.isManaging()) {
            this->thread = &thread;
            this->gpu = gpu;
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


AbstractTask::ExecutionTarget CustomPipeline::getExecutionTarget() const {
    return impl->getExecutionTarget();
}

ThreadIndex CustomPipeline::maxAllowedThreads() const {
    return impl->maxAllowedThreads();
}

void CustomPipeline::beforeProcessing(ThreadIndex threadCount, GraphicPipeline *gpu) {
    impl->beforeProcessing();
}

void CustomPipeline::afterProcessing(ThreadIndex threadCount, bool aborted) {
    AbstractTask::afterProcessing(threadCount, aborted);
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

CustomPipeline::TaskHolder& CustomPipeline::insertTask(AbstractTask &task, const TaskHolder& goesAfter) {
    TaskHolder* holder = createTaskHolder(task);
    impl->insertTask(holder, &goesAfter);
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

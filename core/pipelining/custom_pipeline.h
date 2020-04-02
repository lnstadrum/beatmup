/*
    Putting multiple tasks into a single one
*/

#pragma once
#include "../parallelism.h"
#include "../exception.h"
#include <vector>
#include <mutex>

namespace Beatmup {
    class TaskRouter;

    class CustomPipeline : public AbstractTask {
    public:
        class TaskHolder;
    private:
        class Impl;
        Impl* impl;

    protected:
        ExecutionTarget getExecutionTarget() const;
        ThreadIndex maxAllowedThreads() const;
        void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, bool aborted);
        bool process(TaskThread& thread);
        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);

        virtual TaskHolder* createTaskHolder(AbstractTask &task) = 0;
        virtual bool route(TaskRouter& router) = 0;

    public:
        CustomPipeline();
        ~CustomPipeline();

        /**
         * Returns number of tasks in the pipeline
         */
        int getTaskCount() const;

        /**
         * Retrieves a task by its index
         */
        TaskHolder& getTask(int index) const;

        /**
         * Retrieves task index if it is in the list; returns -1 otherwise
         */
        int getTaskIndex(const TaskHolder&);

        /**
         * Adds a new task in the end of the pipeline
         */
        TaskHolder& addTask(AbstractTask& task);

        /**
         * Inserts a task in a specified position of the pipeline
         */
        TaskHolder& insertTask(AbstractTask& task, const TaskHolder& succeedingHoder);

        /**
         * Removes a task from the pipeline, if any.
         * \return `true` on success
         */
        bool removeTask(const TaskHolder& task);

        /**
         * Determines pipeline execution mode and thread count
         */
        void measure();

        /**
         * A task within the pipeline
         */
        class TaskHolder : public Object {
        friend class CustomPipeline;
        friend class CustomPipeline::Impl;
        private:
            void operator = (const TaskHolder&) = delete;
        protected:
            AbstractTask& task;
            AbstractTask::ExecutionTarget executionMode;
            ThreadIndex threadCount;
            float time;
            TaskHolder(AbstractTask& task): task(task) {}
            ~TaskHolder() {}
        public:
            TaskHolder(TaskHolder&&);
            AbstractTask& getTask() const { return task; }

            /**
                \return last execution time in milliseconds.
            */
            float getRunTime() const { return time; }
        };

        class PipelineNotReady : public Exception {
        public:
            PipelineNotReady(const char* message): Exception(message) {}
        };
    };


    /**
    * Interface managing the execution of a sequence of tasks
    */
    class TaskRouter {
    public:
        /*
        * Returns currently pointed task
        */
        virtual const CustomPipeline::TaskHolder& getCurrentTask() const = 0;

        /**
        * Executes the pointed task
        */
        virtual void runTask() = 0;

        /**
        * Goes to the next task in the list
        */
        virtual void goToNextTask() = 0;

        /**
        * Returns `true` if all tasks are done
        */
        virtual bool allTasksDone() const = 0;

        /**
        * Returns `true` if the current session is aborted
        */
        virtual bool allTasksAborted() const = 0;
    };

    bool operator == (const CustomPipeline::TaskHolder&, const CustomPipeline::TaskHolder&);
}

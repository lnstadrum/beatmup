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
#include "../parallelism.h"
#include "../exception.h"
#include <vector>
#include <mutex>

namespace Beatmup {
    class TaskRouter;

    /**
        Custom pipeline: a sequence of tasks to be executed as a whole.
        Acts as an AbstractTask. Built by adding tasks one by one and calling measure() at the end.
    */
    class CustomPipeline : public AbstractTask {
    public:
        class TaskHolder;
    private:
        class Impl;
        Impl* impl;

    protected:
        TaskDeviceRequirement getUsedDevices() const;
        ThreadIndex getMaxThreads() const;
        void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
        void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
        bool process(TaskThread& thread);
        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);

        virtual TaskHolder* createTaskHolder(AbstractTask &task) = 0;
        virtual bool route(TaskRouter& router) = 0;

        CustomPipeline();

    public:
        ~CustomPipeline();

        /**
         * \return number of tasks in the pipeline
         */
        int getTaskCount() const;

        /**
         * Retrieves a task by its index
         */
        TaskHolder& getTask(int) const;

        /**
         * Retrieves task index if it is in the pipeline; returns -1 otherwise
         */
        int getTaskIndex(const TaskHolder&);

        /**
         * Adds a new task to the end of the pipeline
         */
        TaskHolder& addTask(AbstractTask&);

        /**
         * Inserts a task in a specified position of the pipeline before another task.
         * \param task          The task to insert
         * \param before        TaskHolder specifying position of the task that will follow the newly inserted task
         * \return TaskHolder with the newly inserted task.
         */
        TaskHolder& insertTask(AbstractTask& task, const TaskHolder& before);

        /**
         * Removes a task from the pipeline.
         * \param task          The task to remove
         * \return `true` on success, `false` if this TaskHolder is not in the pipeline.
         */
        bool removeTask(const TaskHolder& task);

        /**
         * Determines pipeline execution mode and required thread count
         */
        void measure();

        /**
            A task within a pipeline
         */
        class TaskHolder : public Object {
        friend class CustomPipeline;
        friend class CustomPipeline::Impl;
        private:
            void operator = (const TaskHolder&) = delete;
        protected:
            AbstractTask& task;
            AbstractTask::TaskDeviceRequirement executionMode;
            ThreadIndex threadCount;
            float time;
            TaskHolder(AbstractTask& task): task(task) {}
        public:
            TaskHolder(TaskHolder&&);
            ~TaskHolder() {}

            /**
                \return the task in the current holder.
            */
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
        Interface managing the execution of a sequence of tasks
    */
    class TaskRouter {
    public:
        /**
            Returns currently pointed task
        */
        virtual CustomPipeline::TaskHolder& getCurrentTask() = 0;
        virtual const CustomPipeline::TaskHolder& getCurrentTask() const = 0;

        /**
            Executes the pointed task
        */
        virtual void runTask() = 0;

        /**
            Goes to the next task in the list
        */
        virtual void goToNextTask() = 0;

        /**
            Returns `true` if all tasks are done
        */
        virtual bool allTasksDone() const = 0;

        /**
            Returns `true` if the current session is aborted
        */
        virtual bool allTasksAborted() const = 0;
    };

    bool operator == (const CustomPipeline::TaskHolder&, const CustomPipeline::TaskHolder&);
}

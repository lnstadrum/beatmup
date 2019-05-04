/*
	Parallel processing
*/

#pragma once

#include <condition_variable>
#include <thread>
#include <exception>
#include "basic_types.h"

namespace Beatmup {
	typedef unsigned char PoolIndex;					//!< number of tread pools or a pool index
	typedef unsigned char ThreadIndex;					//!< number of threads / thread index

	
	static const ThreadIndex MAX_THREAD_INDEX = 255;	//!< maximum possible thread index value

	class GraphicPipeline;
	class TaskThread;

	/**
	* A task
	*/
	class AbstractTask : public Object {
	public:
		enum ExecutionTarget {
			doNotUseGPU,				// task does not use GPU
			useGPUIfAvailable,			// task uses GPU if available, CPU otherwise
			useGPU						// task requires GPU
		};

		/**
			Instruction called before the processing starts.
			\param threadCount		selected number of threads to perform the task
			\param gpu				if not NULL, the task will be performed on GPU
		*/
		virtual void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);

		/**
			Instruction called after the processing stops.
			\param threadCount		selected number of threads to perform the task
			\param aborted			`true` if the task was aborted
		*/
		virtual void afterProcessing(ThreadIndex threadCount, bool aborted);

		/**
			Does the job within specified thread.
			\param thread	associated task execution context
			\returns `true` if the execution is finished correctly, `false` otherwise
		*/
		virtual bool process(TaskThread& thread) = 0;

		
		/**
			Does the job within specified thread assuming that GPU is used for this task.
			\param gpu		graphic pipeline interaction interface
			\param thread	associated task execution context
			\returns `true` if the execution is finished correctly, `false` otherwise
		*/
		virtual bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
		
		/**
			\returns task execution mpde
		*/
		virtual ExecutionTarget getExecutionTarget() const;

		/**
			Maximum number of CPU threads the task may be performed in.
		*/
		virtual ThreadIndex maxAllowedThreads() const;

		/**
			Valid thread count from given integer value
		*/
		static ThreadIndex validThreadCount(int N);
	};


	class TaskThread {
		TaskThread(const TaskThread&) = delete;
	protected:
		ThreadIndex current;			//!< current thread index

		inline TaskThread(ThreadIndex current): current(current) {}

	public:
		/**
			\return number of this thread.
		*/
		inline ThreadIndex currentThread() const {
			return current;
		}

		/**
			\return `true` if represents the managing thread
		*/
		inline bool isManaging() const {
			return current == 0;
		}

		/**
			\return overall number of threads working on current task.
		*/
		virtual ThreadIndex totalThreads() const = 0;


		/**
			Returns `true` if the task is asked to stop from outside.
		*/
		virtual bool isTaskAborted() const = 0;


		virtual void synchronize() = 0;
	};

}
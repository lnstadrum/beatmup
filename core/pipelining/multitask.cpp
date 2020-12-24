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
#include "multitask.h"
#include "../debug.h"

using namespace Beatmup;

namespace Internal {
    /**
        TaskHolder implementation for Multitask pipeline.
        Used only internally.
    */
    class MultitaskTaskHolder : public CustomPipeline::TaskHolder {
    public:
        Multitask::RepetitionPolicy repetitionPolicy;

        MultitaskTaskHolder(AbstractTask &task) : TaskHolder(task)
        {
            repetitionPolicy = Multitask::RepetitionPolicy::REPEAT_ALWAYS;
        }
    };
}


CustomPipeline::TaskHolder* Multitask::createTaskHolder(AbstractTask &task) {
    return new Internal::MultitaskTaskHolder(task);
}

bool Multitask::route(TaskRouter &router) {
    // skip tasks that are "ignored"
    {
        std::lock_guard<std::mutex> lock(policyAccess);
        while (!router.allTasksDone()) {
            RepetitionPolicy policy = static_cast<const Internal::MultitaskTaskHolder&>(router.getCurrentTask()).repetitionPolicy;
            if (policy == RepetitionPolicy::REPEAT_ALWAYS || policy == RepetitionPolicy::REPEAT_UPDATE)
                break;
            router.goToNextTask();
        }
    }

    // then go
    while (!router.allTasksDone() && !router.allTasksAborted()) {
        Internal::MultitaskTaskHolder& task = static_cast<Internal::MultitaskTaskHolder&>(router.getCurrentTask());
        if (task.repetitionPolicy != RepetitionPolicy::IGNORE_ALWAYS)
            router.runTask();

        // switch repetition policy if needed
        policyAccess.lock();
        if (task.repetitionPolicy == RepetitionPolicy::REPEAT_UPDATE)
            task.repetitionPolicy = RepetitionPolicy::IGNORE_IF_UPTODATE;
        policyAccess.unlock();

        router.goToNextTask();
    }

    return true;
}

Multitask::Multitask() {}

Multitask::RepetitionPolicy Multitask::getRepetitionPolicy(const TaskHolder &task)
{
    BEATMUP_ASSERT_DEBUG(getTaskIndex(task) >= 0);
    std::lock_guard<std::mutex> lock(policyAccess);
    return ((const Internal::MultitaskTaskHolder&)task).repetitionPolicy;
}

void Multitask::setRepetitionPolicy(TaskHolder &taskHolder, RepetitionPolicy policy) {
    BEATMUP_ASSERT_DEBUG(getTaskIndex(taskHolder) >= 0);
    std::lock_guard<std::mutex> lock(policyAccess);
    static_cast<Internal::MultitaskTaskHolder&>(taskHolder).repetitionPolicy = policy;
}

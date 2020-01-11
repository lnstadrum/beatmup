#include "../exception.h"
#include "multitask.h"
#include "../debug.h"

using namespace Beatmup;

class MultitaskTaskHolder : public CustomPipeline::TaskHolder {
public:
    Multitask::RepetitionPolicy repetitionPolicy;

    MultitaskTaskHolder(AbstractTask &task) : TaskHolder(task)
    {
        repetitionPolicy = Multitask::RepetitionPolicy::REPEAT_ALWAYS;
    }
};

CustomPipeline::TaskHolder* Multitask::createTaskHolder(AbstractTask &task) {
    return new MultitaskTaskHolder(task);
}

bool Multitask::route(TaskRouter &router) {
    // skip tasks that are "ignored"
    {
        std::lock_guard<std::mutex> lock(policyAccess);
        while (!router.allTasksDone()) {
            RepetitionPolicy policy = ((MultitaskTaskHolder&)router.getCurrentTask()).repetitionPolicy;
            if (policy == RepetitionPolicy::REPEAT_ALWAYS || policy == RepetitionPolicy::REPEAT_UPDATE)
                break;
            router.goToNextTask();
        }
    }

    // then go
    while (!router.allTasksDone() && !router.allTasksAborted()) {
        MultitaskTaskHolder& task = (MultitaskTaskHolder&)router.getCurrentTask();
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
    return ((const MultitaskTaskHolder&)task).repetitionPolicy;
}

void Multitask::setRepetitionPolicy(TaskHolder &task, RepetitionPolicy policy) {
    BEATMUP_ASSERT_DEBUG(getTaskIndex(task) >= 0);
    std::lock_guard<std::mutex> lock(policyAccess);
    ((MultitaskTaskHolder&)task).repetitionPolicy = policy;
}

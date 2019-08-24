#include "context_event_listener.h"
#include "log.h"


using namespace Beatmup;

ContextEventListener::ContextEventListener(JNIEnv *jenv) {
    jenv->GetJavaVM(&jvm);
}


void ContextEventListener::threadCreated(PoolIndex pool) {
    // thread attaching is performed every time it is actually needed
}


void ContextEventListener::threadTerminating(PoolIndex pool) {
    // thread detaching is done here
    jvm->DetachCurrentThread();
}


void ContextEventListener::taskFail(PoolIndex pool, Beatmup::AbstractTask &task, const std::exception &ex) {
    LOG_E("BEATMUP GLOBAL EVENT: TASK FAILED in pool %d", pool);
}


void ContextEventListener::gpuInitFail(PoolIndex pool, const std::exception &ex) {
    LOG_E("BEATMUP GLOBAL EVENT: GPU INITIALIZATION FAILED in pool %d", pool);
}


bool ContextEventListener::taskDone(PoolIndex pool, Beatmup::AbstractTask &task, bool aborted) {
    return false;
}
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
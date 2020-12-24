/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include "callback_task.h"

using namespace Beatmup;

const char* CallbackTask::JAVA_CLASS_NAME = "Beatmup/Utils/Callback";
const char* CallbackTask::JAVA_METHOD_NAME = "run";


CallbackTask::CallbackTask(JNIEnv* jenv): callbackObject(nullptr) { 
    jenv->GetJavaVM(&jvm);
    jclass cls = jenv->FindClass(JAVA_CLASS_NAME);
    callbackMethodId = jenv->GetMethodID(cls, JAVA_METHOD_NAME, "()V");
}


CallbackTask::~CallbackTask() {
    if (callbackObject) {
        JNIEnv *jenv;
        jvm->AttachCurrentThread(&jenv, nullptr);
        jenv->DeleteGlobalRef(callbackObject);
    }
}


bool CallbackTask::process(TaskThread& thread) {
    JNIEnv *jenv;
    jvm->AttachCurrentThread(&jenv, nullptr);
    jenv->CallVoidMethod(callbackObject, callbackMethodId);
    return true;
}


void CallbackTask::setCallbackObject(JNIEnv* jenv, jobject obj) {
    if (callbackObject)
        jenv->DeleteGlobalRef(callbackObject);
    callbackObject = jenv->NewGlobalRef(obj);
}
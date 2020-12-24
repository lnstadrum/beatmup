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

#pragma once

#include <jni.h>
#include <core/parallelism.h>

namespace Beatmup {
    /**
        Task calling method of a Java object.
    */
    class CallbackTask : public AbstractTask {
    private:
        JavaVM *jvm;
        jmethodID callbackMethodId;
        jobject callbackObject;

        bool process(TaskThread& thread);

        inline TaskDeviceRequirement getUsedDevices() const {
            return TaskDeviceRequirement::CPU_ONLY;
        }

        inline ThreadIndex getMaxThreads() const {
            return 1;
        }

    public:
        static const char* JAVA_CLASS_NAME;         //!< callback object java class name
        static const char* JAVA_METHOD_NAME;        //!< called method name

        CallbackTask(JNIEnv* jenv);

        ~CallbackTask();

        /**
            Sets the object to be called.
            The task calls a void method without parameters named JAVA_METHOD_NAME of a JAVA_CLASS_NAME class instance.
            The object is protected from accidental garbage collection by a global reference until the task instance is destroyed.
            \param jenv     Java environment instance
            \param obj      The instance to be called
        */
        void setCallbackObject(JNIEnv* jenv, jobject obj);
    };
}
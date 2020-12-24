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

#include <core/context.h>
#include <jni.h>

namespace Beatmup {

    /**
     * Needed for correct functioning of JNI
     */
    class ContextEventListener : public Beatmup::Context::EventListener {
    private:
        JavaVM *jvm;

    public:
        ContextEventListener(JNIEnv *jenv);

        void threadCreated(PoolIndex pool);

        void threadTerminating(PoolIndex pool);

        void taskFail(PoolIndex pool, Beatmup::AbstractTask &task, const std::exception &ex);

        void gpuInitFail(PoolIndex pool, const std::exception &ex);

        bool taskDone(PoolIndex pool, Beatmup::AbstractTask &task, bool aborted);
    };

}
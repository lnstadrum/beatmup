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

#include <core/basic_types.h>

#include "log.h"

#include "java_factory.h"

#include <mutex>
#include <vector>
#include <map>
#include <string>


/**
    Handles all Java binding-related static data.
*/
class BeatmupJavaObjectPool {
private:
    std::multimap<const Beatmup::Object*, jobject> javaRefs;    //!< Java global references representing dependencies of internal objects on external Java objects

    struct {
        JNIEnv* env;
        jfieldID handleFieldId;
    } javaContext;


    /**
        Java context check
    */
    inline bool queryJavaContext(JNIEnv* newEnv) {
        if (javaContext.env == newEnv)
            return true;
        std::lock_guard<std::mutex> lock(access);
        javaContext.env = newEnv;
        jclass cls = newEnv->FindClass("Beatmup/Object");
        javaContext.handleFieldId = newEnv->GetFieldID(cls, "handle", "J");
        newEnv->DeleteLocalRef(cls);
        return false;
    }

    std::mutex access;                              //!< structure access control

public:
    static const jlong INVALID_HANDLE = std::numeric_limits<jlong>::min();

    JavaFactory factory;

    /**
     * Retrieves native object by its handle
     */
    template<class Object> inline Object* getObject(JNIEnv* jenv, jlong handle) {
#ifdef DEBUG_LOGGING
        LOG_I("Hitting object #%lld", handle);
#endif
        if (handle == INVALID_HANDLE)
            return nullptr;
        return static_cast<Object*>( (void*)handle );
    }

    /**
     * Retrieves native object from its Java prototype
     */
    template<class Object> inline Object* getObject(JNIEnv* jenv, jobject obj) {
        queryJavaContext(jenv);
        if (!obj)
            return nullptr;
        jlong ptr = jenv->GetLongField(obj, javaContext.handleFieldId);
#ifdef DEBUG_LOGGING
        LOG_I("Hitting object #%lld from Java", ptr);
#endif
        if (ptr == INVALID_HANDLE)
            return nullptr;
        return static_cast<Object*>( (void*)jenv->GetLongField(obj, javaContext.handleFieldId) );
    }


    /**
     * Sets a handle of a java object to null
     */
    void nullifyHandle(JNIEnv* jenv, jobject obj) {
        queryJavaContext(jenv);
        jenv->SetLongField(obj, javaContext.handleFieldId, INVALID_HANDLE);
    }

    /**
        Creates new global reference on a Java object to avoid garbage collecting
    */
    void addJavaReference(JNIEnv* jenv, jobject jobj, const Beatmup::Object* bobj) {
        std::lock_guard<std::mutex> lock(access);
        javaRefs.insert(std::make_pair(bobj, jenv->NewGlobalRef(jobj)));
    }


    /**
       Removes a Java reference
    */
    void removeJavaReference(JNIEnv* jenv, const Beatmup::Object* bobj) {
        std::lock_guard<std::mutex> lock(access);
        auto ref = javaRefs.find(bobj);
        if (ref != javaRefs.end()) {
            jenv->DeleteGlobalRef(ref->second);
            javaRefs.erase(ref);
        }
    }


    /**
       Removes all Java references to depending objects
    */
    void removeAllJavaReferences(JNIEnv* jenv, const Beatmup::Object* bobj) {
        std::lock_guard<std::mutex> lock(access);
        while (true) {
            auto ref = javaRefs.find(bobj);
            if (ref == javaRefs.end())
                break;
            jenv->DeleteGlobalRef(ref->second);
            javaRefs.erase(ref);
        }
    }


    /**
        Returns a reference to the Java object which a given object depends on.
    */
    jobject getJavaReference(const Beatmup::Object* bobj) const {
        return javaRefs.find(bobj)->second;
    }



    BeatmupJavaObjectPool() {
        javaContext.env = NULL;
    }


    /**
        Throws a specific exception
    */
    static void throwToJava(JNIEnv* jenv, const char* exceptionClass, const char* message) {
        jclass exClass = jenv->FindClass(exceptionClass);
        jmethodID constructor = jenv->GetMethodID(exClass, "<init>", "(Ljava/lang/String;)V");
        jobject exception = jenv->NewObject(exClass, constructor, jenv->NewStringUTF(message));
        jenv->Throw((jthrowable) exception);
        jenv->DeleteLocalRef(exClass);
    }


    /**
        Throws a general exception
    */
    static void throwToJava(JNIEnv* jenv, const char* message) {
        throwToJava(jenv, "Beatmup/Exceptions/CoreException", message);
    }


    /**
        Rethrows a core-caused exception
    */
    static void rethrowToJava(JNIEnv* jenv, std::exception& ex) {
        throwToJava(jenv, ex.what());
    }
};

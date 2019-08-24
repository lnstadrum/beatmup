#pragma once
#include "objectpool.h"

#include "log.h"

#include <mutex>
#include <string>


#define JNIMETHOD(R,N,C,O) JNIEXPORT R JNICALL C##_##O


// locking the mutex
#ifdef DEBUG_LOGGING
#define BEATMUP_ENTER LOG_I("Entering %s : %d", __FILE__, __LINE__)
#else
#define BEATMUP_ENTER
#endif

// catches and rethrows an exception to Java
#define BEATMUP_CATCH(expr) try expr catch (std::exception& ex) { $pool.rethrowToJava(jenv, ex); }

// retrieving Beatmup object by its handle
#define BEATMUP_OBJ(type,ptr,handle) \
    type* ptr = $pool.getObject<type>(jenv, handle)

#define BEATMUP_STRING(var) \
    const char* javaChar = jenv->GetStringUTFChars(var, 0); \
    const std::string name##Str(javaChar); \
    jenv->ReleaseStringUTFChars(var, javaChar)


#define BEATMUP_OBJ_OR_NULL(t,p,h) BEATMUP_OBJ(t,p,h)

// creating a new dependency on a Java object
#define BEATMUP_REFERENCE(jobj,obj) $pool.addJavaReference(jenv, jobj, obj)

// removing a dependency on Java object
#define BEATMUP_DELETE_REFERENCE(obj) if (obj) $pool.removeJavaReference(jenv, obj)


extern BeatmupJavaObjectPool $pool;
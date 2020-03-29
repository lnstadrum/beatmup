#include "exception.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef BEATMUP_PLATFORM_ANDROID
    #include <android/log.h>
#endif

Beatmup::Exception::Exception(const char * message, ...) {
    static const int MAX_LENGTH = 4*1024;
    char out[MAX_LENGTH];
    va_list argptr;
    va_start(argptr, message);
#if _MSC_VER
    vsnprintf_s(out, MAX_LENGTH, message, argptr);
#else
    vsnprintf(out, MAX_LENGTH, message, argptr);
#endif
    va_end(argptr);
    this->message.assign(out);
      
#if BEATMUP_PLATFORM_ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "Beatmup", "%s", out);
#endif
#ifdef BEATMUP_DEBUG
    printf("%s\n", out);
#endif
}


void Beatmup::Insanity::insanity(const char* message) {
    throw Beatmup::Insanity(message);
}


void Beatmup::DebugAssertion::check(bool condition, const char* message, ...) {
    if (condition)
        return;
    static const int MAX_LENGTH = 4*1024;
    char out[MAX_LENGTH];
    va_list argptr;
    va_start(argptr, message);
#if _MSC_VER
    vsnprintf_s(out, MAX_LENGTH, message, argptr);
#else
    vsnprintf(out, MAX_LENGTH, message, argptr);
#endif
    va_end(argptr);
    throw Beatmup::DebugAssertion(out);
}

#include "exception.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef BEATMUP_PLATFORM_ANDROID
	#include <android/log.h>
#endif

Beatmup::Exception::Exception(const char * message, ...) {
	const int MAX_LENGTH = 4*1024;
	char out[MAX_LENGTH];
	va_list argptr;
	va_start(argptr, message);
#if BEATMUP_PLATFORM_WINDOWS
	vsnprintf_s(out, MAX_LENGTH, message, argptr);
	printf("%s\n", out);
#elif BEATMUP_PLATFORM_ANDROID
	vsnprintf(out, MAX_LENGTH, message, argptr);
	__android_log_print(ANDROID_LOG_ERROR, "Beatmup", "%s", out);
#endif
	va_end(argptr);
	this->message.assign(out);
	//TODO: out
}


void Beatmup::Exception::check(bool condition, const char* message) {
	if (!condition)
		throw Beatmup::Exception(message);
}


void Beatmup::NullTaskInput::check(void* pointer, const char* which) {
	if (!pointer)
		throw NullTaskInput(which);
}


void Beatmup::Insanity::insanity(const char* message) {
	throw Beatmup::Insanity(message);
}

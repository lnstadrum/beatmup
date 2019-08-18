#pragma once
#ifdef BEATMUP_DEBUG

	#ifdef BEATMUP_PLATFORM_ANDROID
		#include <android/log.h>
		#define BEATMUP_DEBUG_I(...)  __android_log_print(ANDROID_LOG_INFO,  "Beatmup debugging", __VA_ARGS__)
		#define BEATMUP_DEBUG_E(...)  __android_log_print(ANDROID_LOG_ERROR, "Beatmup debugging", __VA_ARGS__)
	#else
		#define BEATMUP_DEBUG_I(...)  { printf(__VA_ARGS__); printf("\n"); }
		#define BEATMUP_DEBUG_E(...)  { printf(__VA_ARGS__); printf("\n"); }
	#endif

#else

	#define BEATMUP_DEBUG_I(...)
	#define BEATMUP_DEBUG_E(...)

#endif
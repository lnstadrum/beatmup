#pragma once
#if defined(_MSC_VER)
	#define  DEBUG_I(...)  { printf(__VA_ARGS__); printf("\n"); }
	#define  DEBUG_E(...)  { printf(__VA_ARGS__); printf("\n"); }
#else
	#include <android/log.h>
	#define  DEBUG_I(...)  __android_log_print(ANDROID_LOG_INFO,  "Beatmup debugging", __VA_ARGS__)
	#define  DEBUG_E(...)  __android_log_print(ANDROID_LOG_ERROR, "Beatmup debugging", __VA_ARGS__)
#endif



#define START_TIMING	auto startTime = std::chrono::high_resolution_clock::now()
#define STOP_TIMING		printf("%f\n", std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count())
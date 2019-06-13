#pragma once
#ifdef BEATMUP_PLATFORM_ANDROID
	#include <android/log.h>
	#define  DEBUG_I(...)  __android_log_print(ANDROID_LOG_INFO,  "Beatmup debugging", __VA_ARGS__)
	#define  DEBUG_E(...)  __android_log_print(ANDROID_LOG_ERROR, "Beatmup debugging", __VA_ARGS__)
#else
  #define  DEBUG_I(...)  { printf(__VA_ARGS__); printf("\n"); }
  #define  DEBUG_E(...)  { printf(__VA_ARGS__); printf("\n"); }
#endif



//#define START_TIMING	auto startTime = std::chrono::high_resolution_clock::now()
//#define STOP_TIMING		printf("%f\n", std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count())

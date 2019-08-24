#pragma once
#include <android/log.h>

#define  LOG_TAG    "Beatmup Core"
#define  LOG_I(...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define  LOG_E(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
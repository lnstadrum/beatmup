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

#pragma once
#ifdef BEATMUP_DEBUG

    #ifdef BEATMUP_PLATFORM_ANDROID
        #include <android/log.h>
        #define BEATMUP_DEBUG_I(...)  __android_log_print(ANDROID_LOG_INFO,  "Beatmup debugging", __VA_ARGS__)
        #define BEATMUP_DEBUG_E(...)  __android_log_print(ANDROID_LOG_ERROR, "Beatmup debugging", __VA_ARGS__)
    #else
        #define BEATMUP_DEBUG_I(FMT, ...)  printf(FMT "\n" , ##__VA_ARGS__);
        #define BEATMUP_DEBUG_E(FMT, ...)  printf(FMT "\n" , ##__VA_ARGS__);
    #endif

#else

    #define BEATMUP_DEBUG_I(...)
    #define BEATMUP_DEBUG_E(...)

#endif
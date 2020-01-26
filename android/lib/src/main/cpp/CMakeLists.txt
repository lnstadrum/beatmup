# This build script ensures compilation of beatmup library for android
#
#   PROFILE_AUDIO and PROFILE_NNETS variables control the inclusion of the corresponding code.
#   USE_JNI enables generating the JNI layer (default is true).

cmake_minimum_required(VERSION 3.2.1)

set(BEATMUP_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
set(PLATFORM_ANDROID ON)

# checking OpenGL ES version
set(USE_EGL YES)
if (GLES_VERSION)
    message(WARNING "OpenGL version is forced to '${GLES_VERSION}'. This may cause compilation issues.")
else()
    set(GLES_VERSION 31)
endif()

# including the core
if(NOT DEFINED USE_JNI)
    set(USE_JNI YES)
endif()

if (USE_JNI)
    include("${BEATMUP_ROOT_DIR}/jni/CMakeLists.in")
else()
    include("${BEATMUP_ROOT_DIR}/core/CMakeLists.in")
endif()

# setting up the library
add_library(beatmup SHARED ${BEATMUP_SOURCES})

# finding libraries
find_library(path_m            m)
find_library(path_android      android)
find_library(path_log          log)
if (PROFILE_AUDIO)
    find_library(path_OpenSLES OpenSLES)
    find_library(path_AAudio aaudio)
endif()

# platform-specific defines
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DBEATMUP_PLATFORM_ANDROID")

if ((ANDROID_ABI STREQUAL "arm64-v8a") OR (ANDROID_ABI STREQUAL "x86_64"))
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DBEATMUP_PLATFORM_64BIT_")
endif()

# LINKING
target_link_libraries(beatmup
    ${path_m}
    ${path_android}
    ${path_log}
)

if (PROFILE_AUDIO)
    target_link_libraries(beatmup ${path_OpenSLES} ${path_AAudio})
endif()

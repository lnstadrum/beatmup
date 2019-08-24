cmake_minimum_required(VERSION 3.2.1)

set(BEATMUP_ROOT_DIR "../../../../..")
set(PLATFORM_ANDROID ON)

# checking OpenGL ES version
set(USE_EGL YES)
if (GLES_VERSION)
    message(WARNING "OpenGL version is forced to '${GLES_VERSION}'. This may cause compilation issues.")
else()
    set(GLES_VERSION 31)
endif()


# including the core
include("${BEATMUP_ROOT_DIR}/jni/CMakeLists.in")


# setting up the library
add_library(beatmup SHARED ${BEATMUP_SOURCES})



# finding libraries
find_library(path_m            m)
find_library(path_android      android)
find_library(path_log          log)
find_library(path_jnigraphics  jnigraphics)


if (PROFILE_AUDIO)
    find_library(path_OpenSLES OpenSLES)
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
    ${path_jnigraphics}
)

if (PROFILE_AUDIO)
    target_link_libraries(beatmup
        ${path_OpenSLES}
    )
endif()

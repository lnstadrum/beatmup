#
#    Beatmup image and signal processing library
#    Copyright (C) 2019, lnstadrum
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# This build script ensures compilation of beatmup library for android
#
#   PROFILE_AUDIO and PROFILE_NNETS variables control the inclusion of the corresponding code.
#   USE_JNI enables generating the JNI layer (default is true).

cmake_minimum_required(VERSION 3.2.1)

set(BEATMUP_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
set(PLATFORM_ANDROID ON)

# checking OpenGL ES version
option(USE_EGL "Use EGL backend" ON)
if (GLES_VERSION)
    message(WARNING "OpenGL version is forced to '${GLES_VERSION}'. This may cause compilation issues.")
else()
    set(GLES_VERSION 31)
endif()

# including the core
option(USE_JNI "Include JNI" ON)
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
find_library(path_jnigraphics  jnigraphics)
find_library(path_log          log)
if (PROFILE_AUDIO)
    find_library(path_OpenSLES OpenSLES)
    if (ANDROID_PLATFORM GREATER 26)
        find_library(path_AAudio aaudio)
    endif()
endif()

# platform-specific defines
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DBEATMUP_PLATFORM_ANDROID")

if ((ANDROID_ABI STREQUAL "arm64-v8a") OR (ANDROID_ABI STREQUAL "x86_64"))
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DBEATMUP_PLATFORM_64BIT_")
endif()

# headers generation
add_custom_target(jniheaders
    COMMAND if [ -d ${BEATMUP_ROOT_DIR}/android/lib/build/intermediates/classes/full/release ]; then
        export [=[SDK_PATH=$${ANDROID_HOME:-$$HOME/Android/Sdk}]=] &&
        javah -d "${BEATMUP_ROOT_DIR}/jni/include"
            -classpath "$$SDK_PATH/platforms/${ANDROID_PLATFORM}/android.jar:${BEATMUP_ROOT_DIR}/android/lib/build/intermediates/classes/full/release"
            Beatmup.Object
            Beatmup.Context
            Beatmup.Android.Context
            Beatmup.Android.ExternalBitmap
            Beatmup.Sequence
            Beatmup.Visual.Android.BasicDisplay
            Beatmup.Rendering.Scene
            Beatmup.Rendering.SceneRenderer
            Beatmup.Shading.ImageShader
            Beatmup.Shading.ShaderApplicator
            Beatmup.Bitmap
            Beatmup.Audio.Signal
            Beatmup.Audio.SignalPlot
            Beatmup.Audio.Playback
            Beatmup.Audio.HarmonicSource
            Beatmup.Imaging.BinaryOperation
            Beatmup.Imaging.ColorMatrix
            Beatmup.Imaging.FloodFill
            Beatmup.Imaging.Filters.PixelwiseFilter
            Beatmup.Imaging.Filters.ColorMatrixTransform
            Beatmup.Imaging.Filters.Sepia
            Beatmup.Imaging.Resampler
            Beatmup.NNets.AbstractOperation
            Beatmup.NNets.DeserializedModel
            Beatmup.NNets.InferenceTask
            Beatmup.NNets.ImageSampler
            Beatmup.NNets.Model
            Beatmup.NNets.Softmax
            Beatmup.Pipelining.CustomPipeline
            Beatmup.Pipelining.Multitask
            Beatmup.Pipelining.TaskHolder
            Beatmup.Utils.Callback
            Beatmup.Utils.ChunkAsset
            Beatmup.Utils.ChunkCollection
            Beatmup.Utils.ChunkFile
            Beatmup.Utils.VariablesBundle
        \; fi
    COMMENT "Generating JNI header files...")

add_dependencies(beatmup jniheaders)

# LINKING
target_link_libraries(beatmup
    ${path_m}
    ${path_android}
    ${path_jnigraphics}
    ${path_log}
)

if (PROFILE_AUDIO)
    target_link_libraries(beatmup ${path_OpenSLES})
    if (ANDROID_PLATFORM GREATER 26)
        target_link_libraries(beatmup ${path_AAudio})
    endif()
endif()

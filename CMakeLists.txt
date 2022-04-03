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
cmake_minimum_required(VERSION 3.2.1)
project (Apps)

option(USE_LIB "Build static library and use it for all apps instead compiling the Beatmup source code for every app" ON)
option(DEBUG   "Enable debugging" OFF)

set(PROFILE_PLATFORM_SPECIFIC_BITMAP ON)
set(PROFILE_NNETS ON)
include("core/CMakeLists.in")


if (USE_LIB)
    add_library(_beatmup STATIC ${BEATMUP_SOURCES})
    if (USE_GLX OR USE_OPENGL)
        add_dependencies(_beatmup libglewmx_static)
        if (NOT MSVC)
            target_compile_options(libglewmx_static PRIVATE "-fPIC")
        endif()
    endif()
endif (USE_LIB)

# compiler-specific flags (not for Visual Studio)
if (NOT MSVC)
    # add stop-on-first-error flag
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wfatal-errors")

    # enable position-independend code
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# set debugging-related flags
if (DEBUG)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -DBEATMUP_DEBUG")
elseif()
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")
endif (DEBUG)

######################################################
#                        APPS                        #
######################################################

function(add_app APP_NAME APP_SOURCES)
    if (USE_LIB)
        add_executable(${APP_NAME} ${APP_SOURCES})
        target_link_libraries(${APP_NAME} _beatmup)
    else()
        add_executable(${APP_NAME} ${BEATMUP_SOURCES} ${APP_SOURCES})
    endif (USE_LIB)

    if (MSVC)
        set_property(TARGET ${APP_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BEATMUP_ROOT_DIR})
    elseif((USE_GLX OR USE_OPENGL) AND NOT USE_LIB)
        add_dependencies(${APP_NAME} libglewmx_static)
    endif (MSVC)
endfunction()

add_app(BasicRendering    apps/basic_rendering/app.cpp)
add_app(Benchmark         apps/benchmark/app.cpp)
add_app(Classify          apps/classify/app.cpp)
add_app(FloodFill         apps/flood_fill/app.cpp)
add_app(Shaderer          apps/shaderer/app.cpp)
add_app(Tests             apps/tests/app.cpp)
add_app(Tools             apps/tools/app.cpp)
add_app(X2                apps/x2/app.cpp)


######################################################
#                   PYTHON BINDING                   #
######################################################

# Find necessary packages
find_package( PythonLibs REQUIRED )
include_directories( ${PYTHON_INCLUDE_DIRS} )

add_subdirectory(pybind11)
pybind11_add_module(beatmup SHARED
    python/src/bitmap.cpp
    python/src/chunk_collection.cpp
    python/src/bindings.cpp
    python/src/binding_tools.cpp)
target_link_libraries(beatmup PUBLIC _beatmup)

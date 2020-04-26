cmake_minimum_required(VERSION 3.2.1)
project (Apps)

option(USE_LIB "Build static library and use it for all apps instead compiling the Beatmup source code for every app" ON)


function(add_app APP_NAME APP_SOURCES)

    if (USE_LIB)
        add_executable(${APP_NAME} ${APP_SOURCES})
        target_link_libraries(${APP_NAME} beatmup)
    else()
        add_executable(${APP_NAME} ${BEATMUP_SOURCES} ${APP_SOURCES})
    endif (USE_LIB)

    if (WIN32)
        set_property(TARGET ${APP_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BEATMUP_ROOT_DIR})
    elseif(USE_GLX AND NOT USE_LIB)
        add_dependencies(${APP_NAME} libglewmx_static)
    endif (WIN32)

endfunction()

set(PROFILE_PLATFORM_SPECIFIC_BITMAP ON)
include("../core/CMakeLists.in")

if (USE_LIB)
    add_library(beatmup STATIC ${BEATMUP_SOURCES})
endif (USE_LIB)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")

add_app(BasicRendering    basic_rendering/app.cpp)
add_app(FloodFill         flood_fill/app.cpp)
add_app(Shaderer          shaderer/app.cpp)
add_app(Tools             tools/app.cpp)
add_app(X2                x2/app.cpp)

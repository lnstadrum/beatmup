cmake_minimum_required(VERSION 3.2.1)
project (BasicApp)

set(PROFILE_PLATFORM_SPECIFIC_BITMAP ON)
include("../../core/CMakeLists.txt")

add_executable(BasicApp ${BEATMUP_SOURCES} app.cpp)
target_link_libraries(BasicApp glew$<$<CONFIG:Debug>:d> gdiplus ${OPENGL_LIBRARY})
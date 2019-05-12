cmake_minimum_required(VERSION 3.2.1)
project (Apps)

function(add_app APP_NAME APP_SOURCES)
	add_executable(${APP_NAME} ${BEATMUP_SOURCES} ${APP_SOURCES})
	if (WIN32)
		target_link_libraries(${APP_NAME} gdiplus)
	endif (WIN32)
	target_link_libraries(${APP_NAME} glew$<$<CONFIG:Debug>:d> ${OPENGL_LIBRARY})
endfunction()

set(PROFILE_PLATFORM_SPECIFIC_BITMAP ON)
include("../core/CMakeLists.in")


add_app(BasicRendering basic_rendering/app.cpp)
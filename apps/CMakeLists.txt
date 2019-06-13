cmake_minimum_required(VERSION 3.2.1)
project (Apps)

function(add_app APP_NAME APP_SOURCES)
	add_executable(${APP_NAME} ${BEATMUP_SOURCES} ${APP_SOURCES})
	if (WIN32)
		target_link_libraries(${APP_NAME} gdiplus)
		set_property(TARGET ${APP_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BEATMUP_ROOT_DIR})
		target_link_libraries(${APP_NAME} glew$<$<CONFIG:Debug>:d>)
	else()
		add_dependencies(${APP_NAME} libglewmx_static)
		find_package(Threads REQUIRED)
		find_package(X11 REQUIRED)
		target_link_libraries(${APP_NAME} ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} glewmx)
	endif (WIN32)
	target_link_libraries(${APP_NAME} ${OPENGL_LIBRARY})
endfunction()

set(PROFILE_PLATFORM_SPECIFIC_BITMAP ON)
include("../core/CMakeLists.in")


add_app(BasicRendering    basic_rendering/app.cpp)
add_app(FloodFill         flood_fill/app.cpp)

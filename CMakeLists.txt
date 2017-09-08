cmake_minimum_required(VERSION 3.6)

project(polar)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include_directories(include)

add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -D_CRT_SECURE_NO_WARNINGS)

set(POLAR_SOURCES
	src/polar/core/debugmanager.cpp
	src/polar/util/buildinfo.cpp
	src/polar/system/audio.cpp
	src/polar/system/renderer/gl32.cpp
	src/polar/system/player/human.cpp
	src/polar/system/input.cpp
	src/polar/system/integrator.cpp
	src/polar/system/work.cpp
	src/polar/support/work/worker.cpp
	src/glew.c
	src/freefall.cpp
	src/main-freefall.cpp
)

set(ASSETBUILDER_SOURCES
	src/polar/core/debugmanager.cpp
	src/polar/util/buildinfo.cpp
	src/main-assetbuilder.cpp
)

if(WIN32)
	set(POLAR_LIBS
		legacy_stdio_definitions.lib
		shlwapi.lib
	)
elseif(APPLE)
	find_library(CARBON_LIB Carbon)
	find_library(AUDIOTOOLBOX_LIB AudioToolbox)
	find_library(AUDIOUNIT_LIB AudioUnit)
	find_library(COREAUDIO_LIB CoreAudio)
	set(POLAR_LIBS
		${CARBON_LIB}
		${AUDIOTOOLBOX_LIB}
		${AUDIOUNIT_LIB}
		${COREAUDIO_LIB}
	)
else()
endif()

if(UNIX)
	find_library(PTHREAD_LIB pthread)
	set(POLAR_LIBS ${POLAR_LIBS}
		${PTHREAD_LIB}
	)
endif()

if(WIN32)
	set(LIB_SEARCH_PATH lib/win32)
elseif(APPLE)
	set(LIB_SEARCH_PATH lib/macos)
else()
	set(LIB_SEARCH_PATH lib/linux64)
endif()

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_library(Z_LIB NAMES z zlib      PATHS ${LIB_SEARCH_PATH})
find_library(PORTAUDIO_LIB portaudio PATHS ${LIB_SEARCH_PATH})
find_library(STEAM_API_LIB steam_api PATHS ${LIB_SEARCH_PATH})

set(POLAR_LIBS ${POLAR_LIBS}
	${PORTAUDIO_LIB}
	${OPENGL_LIBRARIES}
	${SDL2_LIBRARY}
	${TTF_LIBRARY}
	${STEAM_API_LIB}
)

if(WIN32)
	find_file(STEAM_API_DYLIB "steam_api.dll" PATH_SUFFIXES ${LIB_SEARCH_PATH} PATHS ${CMAKE_SOURCE_DIR})
elseif(APPLE)
	set(STEAM_API_DYLIB ${STEAM_API_LIB})
else()
	find_file(STEAM_API_DYLIB "steam_api.so" PATH_SUFFIXES ${LIB_SEARCH_PATH} PATHS ${CMAKE_SOURCE_DIR})
endif()

set(POLAR_FRAMEWORKS ${POLAR_LIBS})
list(FILTER POLAR_FRAMEWORKS INCLUDE REGEX "\.framework$")
list(FILTER POLAR_FRAMEWORKS EXCLUDE REGEX "^/System/")

set(STEAM_APPID 0 CACHE STRING "Steam application ID")
if(NOT STEAM_APPID GREATER "0")
	message(FATAL_ERROR "STEAM_APPID variable must be non-zero.")
endif()

set(STEAM_APPID_FILE ${CMAKE_CURRENT_BINARY_DIR}/steam_appid.txt)
file(WRITE ${STEAM_APPID_FILE} "${STEAM_APPID}")

set(APP_RESOURCES
	${STEAM_API_DYLIB}
	${STEAM_APPID_FILE}
)

set(RESOURCES
)

# Asset Builder

set(ASSETBUILDER_LIBS
	${WIN32_LIBS}
	${APPLE_LIBS}
	${SDL2_LIBRARY}
	${Z_LIB}
)

add_executable(assetbuilder ${ASSETBUILDER_SOURCES})
target_link_libraries(assetbuilder ${ASSETBUILDER_LIBS})
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD 11)
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD_REQUIRED ON)

# Polar Engine
add_executable(polar WIN32 MACOSX_BUNDLE ${POLAR_SOURCES})
target_link_libraries(polar ${POLAR_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 11)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)

if(APPLE)
	set(POLAR_EXECUTABLE_DIR ${CMAKE_CURRENT_BINARY_DIR}/polar.app/Contents/MacOS)
	set(POLAR_RESOURCES_DIR  ${CMAKE_CURRENT_BINARY_DIR}/polar.app/Contents/Resources)
	set(POLAR_FRAMEWORKS_DIR ${CMAKE_CURRENT_BINARY_DIR}/polar.app/Contents/Frameworks)
	add_custom_command(TARGET polar POST_BUILD
		COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/../Frameworks" $<TARGET_FILE:polar>
	)
else()
	set(POLAR_EXECUTABLE_FILE $<TARGET_FILE:polar> CACHE INTERNAL "")
	set(POLAR_EXECUTABLE_DIR $<TARGET_FILE_DIR:polar> CACHE INTERNAL "")
	set(POLAR_RESOURCES_DIR ${POLAR_EXECUTABLE_DIR} CACHE INTERNAL "")
endif()

set(APP_FRAMEWORKS_CMDS "")
foreach(V ${POLAR_FRAMEWORKS})
	get_filename_component(BARENAME ${V} NAME)
	set(APP_FRAMEWORKS_CMDS ${APP_FRAMEWORKS_CMDS} COMMAND ${CMAKE_COMMAND} -E copy_directory ${V} ${POLAR_FRAMEWORKS_DIR}/${BARENAME})
endforeach()
add_custom_target(appframeworks ALL ${APP_FRAMEWORKS_CMDS})
message(STATUS "${CMDS}")

# App Resources
if(APP_RESOURCES)
	add_custom_target(appresources ALL
		COMMAND ${CMAKE_COMMAND} -E copy ${APP_RESOURCES} ${POLAR_EXECUTABLE_DIR}
	)
endif()

# Resources
if(RESOURCES)
	add_custom_target(resources ALL
		COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCES} ${POLAR_RESOURCES_DIR}
	)
endif()

# Assets
add_custom_target(assets ALL
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${POLAR_RESOURCES_DIR}/assets
	COMMAND $<TARGET_FILE:assetbuilder> ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/assets ${POLAR_RESOURCES_DIR}/assets
)

add_dependencies(assets polar)
add_dependencies(assets assetbuilder)

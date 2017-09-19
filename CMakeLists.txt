cmake_minimum_required(VERSION 3.6)

project(polar)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include_directories(include)

add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -D_CRT_SECURE_NO_WARNINGS)

set(POLAR_SRCS
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
)

set(ASSETBUILDER_SRCS
	src/assetbuilder/main.cpp
)

set(PONG_SRCS
	src/pong/app.cpp
	src/pong/main.cpp
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

find_package(GLM REQUIRED)
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

set(ASSETBUILDER_LIBS
	${WIN32_LIBS}
	${APPLE_LIBS}
	${SDL2_LIBRARY}
	${Z_LIB}
	$<TARGET_FILE:polar>
)

set(PONG_LIBS
	${POLAR_LIBS}
	$<TARGET_FILE:polar>
)

if(WIN32)
	find_file(STEAM_API_DYLIB "steam_api.dll" PATH_SUFFIXES ${LIB_SEARCH_PATH} PATHS ${CMAKE_CURRENT_SOURCE_DIR})
elseif(APPLE)
	set(STEAM_API_DYLIB ${STEAM_API_LIB})
else()
	find_file(STEAM_API_DYLIB "steam_api.so" PATH_SUFFIXES ${LIB_SEARCH_PATH} PATHS ${CMAKE_CURRENT_SOURCE_DIR})
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

# Asset Builder
add_executable(assetbuilder ${ASSETBUILDER_SRCS})
target_include_directories(assetbuilder PRIVATE ${GLM_INCLUDE_DIR})
target_link_libraries(assetbuilder ${ASSETBUILDER_LIBS})
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD 11)
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD_REQUIRED ON)

# Polar Engine
add_library(polar STATIC ${POLAR_SRCS})
target_include_directories(polar PRIVATE ${GLM_INCLUDE_DIR})
target_link_libraries(polar ${POLAR_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 11)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)

# Pong Example
add_executable(pong WIN32 MACOSX_BUNDLE EXCLUDE_FROM_ALL ${PONG_SRCS})
target_include_directories(polar PRIVATE ${GLM_INCLUDE_DIR})
target_link_libraries(pong ${PONG_LIBS})
set_property(TARGET pong PROPERTY CXX_STANDARD 11)
set_property(TARGET pong PROPERTY CXX_STANDARD_REQUIRED ON)

add_custom_command(TARGET pong POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${STEAM_API_DYLIB} $<TARGET_FILE_DIR:pong>
)

add_dependencies(assetbuilder polar)
add_dependencies(pong polar)

set(POLAR_LIBS ${POLAR_LIBS} PARENT_SCOPE)
set(POLAR_FRAMEWORKS ${POLAR_FRAMEWORKS} PARENT_SCOPE)

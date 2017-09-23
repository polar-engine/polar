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
	src/polar/system/input.cpp
	src/polar/system/integrator.cpp
	src/polar/system/work.cpp
	src/polar/support/work/worker.cpp
	src/glew.c
)

set(ASSETBUILDER_SRCS
	src/assetbuilder/main.cpp
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
find_package(SDL2TTF REQUIRED)
find_library(Z_LIB NAMES z zlib      PATHS ${LIB_SEARCH_PATH})
find_library(PORTAUDIO_LIB portaudio PATHS ${LIB_SEARCH_PATH})

set(POLAR_INCLUDE_DIRS
	${GLM_INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
	${SDL2TTF_INCLUDE_DIR}
)

set(POLAR_LIBS ${POLAR_LIBS}
	${PORTAUDIO_LIB}
	${OPENGL_LIBRARIES}
	${SDL2_LIBRARY}
	${SDL2TTF_LIBRARY}
)

set(POLAR_DYLIBS
	${SDL2_DYLIB}
	${SDL2TTF_DYLIBS}
)

set(ASSETBUILDER_LIBS
	${WIN32_LIBS}
	${APPLE_LIBS}
	${SDL2_LIBRARY}
	${Z_LIB}
	$<TARGET_FILE:polar>
)

set(POLAR_FRAMEWORKS ${POLAR_LIBS})
list(FILTER POLAR_FRAMEWORKS INCLUDE REGEX "\.framework$")
list(FILTER POLAR_FRAMEWORKS EXCLUDE REGEX "^/System/")

# Polar Engine
add_library(polar STATIC ${POLAR_SRCS})
target_include_directories(polar PRIVATE ${POLAR_INCLUDE_DIRS})
target_link_libraries(polar ${POLAR_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 11)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)

# Asset Builder
add_executable(assetbuilder ${ASSETBUILDER_SRCS})
target_include_directories(assetbuilder PRIVATE ${POLAR_INCLUDE_DIRS})
target_link_libraries(assetbuilder ${ASSETBUILDER_LIBS})
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD 11)
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD_REQUIRED ON)
add_dependencies(assetbuilder polar)

if(POLAR_DYLIBS)
	add_custom_command(TARGET assetbuilder POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${POLAR_DYLIBS} $<TARGET_FILE_DIR:assetbuilder>
	)
endif()

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
	set(POLAR_INCLUDE_DIRS ${POLAR_INCLUDE_DIRS} PARENT_SCOPE)
	set(POLAR_LIBS         ${POLAR_LIBS} PARENT_SCOPE)
	set(POLAR_DYLIBS       ${POLAR_DYLIBS} PARENT_SCOPE)
	set(POLAR_FRAMEWORKS   ${POLAR_FRAMEWORKS} PARENT_SCOPE)
endif()

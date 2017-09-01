cmake_minimum_required(VERSION 3.2)

project(polar)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include_directories(include)
link_directories(lib)

add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS)

set(POLAR_SOURCES
	src/DebugManager.cpp
	src/buildinfo.cpp
	Sub-Terra/src/AudioManager.cpp
	Sub-Terra/src/Freefall.cpp
	Sub-Terra/src/GL32Renderer.cpp
	Sub-Terra/src/HumanPlayerController.cpp
	Sub-Terra/src/InputManager.cpp
	Sub-Terra/src/Integrator.cpp
	Sub-Terra/src/JobManager.cpp
	Sub-Terra/src/Worker.cpp
	Sub-Terra/src/glew.c
	Sub-Terra/src/main.cpp
)

set(ASSETBUILDER_SOURCES
	src/DebugManager.cpp
	src/buildinfo.cpp
	AssetBuilder/src/main.cpp
)

if(WIN32)
	set(WIN32_LIBS
		legacy_stdio_definitions.lib
		shlwapi.lib
	)
endif(WIN32)

if(APPLE)
	find_library(CARBON_LIB Carbon)
	find_library(AUDIOTOOLBOX_LIB AudioToolbox)
	find_library(AUDIOUNIT_LIB AudioUnit)
	find_library(COREAUDIO_LIB CoreAudio)

	set(APPLE_LIBS
		${CARBON_LIB}
		${AUDIOTOOLBOX_LIB}
		${AUDIOUNIT_LIB}
		${COREAUDIO_LIB}
	)
endif(APPLE)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_library(Z_LIB NAMES z zlib PATHS lib lib/win32)
find_library(PORTAUDIO_LIB portaudio PATHS lib lib/win32)
find_library(STEAM_API_LIB steam_api PATHS lib lib/win32)

if(WIN32)
	find_file(STEAM_API_DYLIB "steam_api.dll" PATH_SUFFIXES lib/win32 lib PATHS ${CMAKE_SOURCE_DIR})
elseif(APPLE)
	set(STEAM_API_DYLIB ${STEAM_API_LIB})
endif()

set(POLAR_LIBS
	${WIN32_LIBS}
	${APPLE_LIBS}
	${PORTAUDIO_LIB}
	${OPENGL_LIBRARIES}
	${SDL2_LIBRARY}
	${TTF_LIBRARY}
	${STEAM_API_LIB}
)

set(STEAM_APPID 0 CACHE STRING "Steam application ID")

set(STEAM_APPID_FILE ${CMAKE_CURRENT_BINARY_DIR}/steam_appid.txt)
file(WRITE ${STEAM_APPID_FILE} "${STEAM_APPID}")

set(APP_RESOURCES
	${STEAM_API_DYLIB}
	${STEAM_APPID_FILE}
)

set(RESOURCES
)

if(APPLE)
	set(POLAR_EXECUTABLE_DIR ${CMAKE_CURRENT_BINARY_DIR}/polar.app/Contents/MacOS)
	set(POLAR_RESOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}/polar.app/Contents/Resources)
else()
	set(POLAR_EXECUTABLE_DIR ${CMAKE_CURRENT_BINARY_DIR})
	set(POLAR_RESOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Asset Builder

set(ASSETBUILDER_LIBS
	${WIN32_LIBS}
	${APPLE_LIBS}
	${Z_LIB}
)

add_executable(assetbuilder ${ASSETBUILDER_SOURCES})
target_include_directories(assetbuilder PUBLIC AssetBuilder/include)
target_link_libraries(assetbuilder ${ASSETBUILDER_LIBS})
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD 11)
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD_REQUIRED ON)

# Polar Engine

add_executable(polar WIN32 MACOSX_BUNDLE ${POLAR_SOURCES})
target_include_directories(polar PUBLIC Sub-Terra/include)
target_link_libraries(polar ${POLAR_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 11)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)

# App Resources
if(APP_RESOURCES)
	add_custom_target(appresources ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${APP_RESOURCES} ${POLAR_EXECUTABLE_DIR}
	)
endif()

# Resources
if(RESOURCES)
	add_custom_target(resources ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RESOURCES} ${POLAR_RESOURCES_DIR}
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

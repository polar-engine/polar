cmake_minimum_required(VERSION 3.2)

project(polar)

include_directories(include Sub-Terra/include)
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

if(APPLE)
	find_library(CARBON_LIB Carbon)
	find_library(AUDIOUNIT_LIB AudioUnit)
	find_library(AUDIOTOOLBOX_LIB AudioToolbox)
	find_library(COREAUDIO_LIB CoreAudio)
endif(APPLE)

find_library(OPENGL_LIB OpenGL)
find_library(SDL2_LIB SDL2 PATHS lib)
find_library(SDL2_TTF_LIB SDL2_ttf PATHS lib)
find_library(PORTAUDIO_LIB portaudio PATHS lib)
find_library(STEAM_API_LIB steam_api PATHS lib)

set(POLAR_LIBS
	${CARBON_LIB}
	${AUDIOTOOLBOX_LIB}
	${AUDIOUNIT_LIB}
	${COREAUDIO_LIB}
	${SDL2_LIB}
	${SDL2_TTF_LIB}
	${OPENGL_LIB}
	${PORTAUDIO_LIB}
	${STEAM_API_LIB}
)

set(STEAM_APPID 0 CACHE STRING "Steam application ID")
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/steam_appid.txt CONTENT "${STEAM_APPID}")

set(APP_RESOURCES
	${STEAM_API_LIB}
	${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/steam_appid.txt
)

set(RESOURCES
)

set_source_files_properties(${APP_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS)
set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

add_executable(polar WIN32 MACOSX_BUNDLE ${POLAR_SOURCES} ${APP_RESOURCES} ${RESOURCES})
target_link_libraries(polar ${POLAR_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 11)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)

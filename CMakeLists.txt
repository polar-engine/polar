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
	find_library(CARBON_LIBRARY Carbon)
	find_library(SDL2_LIBRARY SDL2)
	find_library(SDL2_TTF_LIBRARY SDL2_ttf)
	find_library(OPENGL_LIBRARY OpenGL)
	set(EXTRA_LIBS
		${CARBON_LIBRARY}
		${SDL2_LIBRARY}
		${SDL2_TTF_LIBRARY}
		${OPENGL_LIBRARY}
	)
endif(APPLE)

add_executable(polar ${POLAR_SOURCES})
target_link_libraries(polar ${EXTRA_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 11)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)

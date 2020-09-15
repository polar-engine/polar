cmake_minimum_required(VERSION 3.6)

project(polar)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
	string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
	if(flag_already_set EQUAL -1)
		check_cxx_compiler_flag("${flag}" flag_supported)
		if(flag_supported)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
		endif()
		unset(flag_supported CACHE)
	endif()
endfunction()

if(WIN32)
else()
	enable_cxx_compiler_flag_if_supported("-Wall")
endif()
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-Werror")

include_directories(include)

add_definitions(
	-DGLEW_STATIC
	-D__STDC_CONSTANT_MACROS
	-D__STDC_LIMIT_MACROS
	-D_CRT_SECURE_NO_WARNINGS
	-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
	-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
)

set(POLAR_SRCS
	src/polar/core/log.cpp
	src/polar/core/polar.cpp
	src/polar/core/ref.cpp
	src/polar/core/state.cpp
	src/polar/asset/font.cpp
	src/polar/asset/level.cpp
	src/polar/component/sprite/box.cpp
	src/polar/component/sprite/slider.cpp
	src/polar/system/audio.cpp
	src/polar/system/integrator.cpp
	src/polar/system/phys.cpp
	src/polar/system/work.cpp
	src/polar/support/work/worker.cpp
	src/polar/fs/local.cpp
	src/polar/util/buildinfo.cpp
)

set(ASSETBUILDER_SRCS
	src/assetbuilder/main.cpp
)

if(WIN32)
	set(WIN32_LIBS
		legacy_stdio_definitions.lib
		shlwapi.lib
	)
	set(POLAR_LIBS ${WIN32_LIBS})
elseif(APPLE)
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
	set(POLAR_LIBS ${APPLE_LIBS})
else()
endif()

if(UNIX)
	find_library(PTHREAD_LIB pthread)
	set(POLAR_LIBS ${POLAR_LIBS}
		${PTHREAD_LIB}
	)
endif()

if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
		set(LIB_SEARCH_PATH lib/win64)
	else()
		set(LIB_SEARCH_PATH lib/win32)
	endif()
elseif(APPLE)
	set(LIB_SEARCH_PATH lib/macos)
else()
	set(LIB_SEARCH_PATH lib/linux64)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

if(WIN32)
	find_package(Boost COMPONENTS system REQUIRED)
elseif(UNIX)
	find_package(Boost COMPONENTS system thread REQUIRED)
else()
endif()
find_package(GLM REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2TTF REQUIRED)
find_package(OpenVR REQUIRED)
find_package(Vulkan REQUIRED)
find_library(Z_LIB NAMES z zlib      PATHS ${LIB_SEARCH_PATH})
find_library(PORTAUDIO_LIB portaudio PATHS ${LIB_SEARCH_PATH})

set(POLAR_INCLUDE_DIRS
	${Boost_INCLUDE_DIRS}
	${GLM_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
	${SDL2TTF_INCLUDE_DIR}
	${OpenVR_INCLUDE_DIR}
	${VULKAN_INCLUDE_DIR}
)

set(POLAR_LIBRARY_DIRS
	${Boost_LIBRARY_DIRS}
)

set(POLAR_LIBS ${POLAR_LIBS}
	${Boost_LIBRARIES}
	${PORTAUDIO_LIB}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${SDL2_LIBRARY}
	${SDL2TTF_LIBRARY}
	${OpenVR_LIBRARY}
	${VULKAN_LIBRARY}
)

set(POLAR_DYLIBS
	${GLEW_DYLIB}
	${SDL2_DYLIB}
	${SDL2TTF_DYLIBS}
	${OpenVR_DYLIB}
	${VULKAN_DYLIB}
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

# invalidate buildinfo
add_custom_target(invalidate_buildinfo ALL
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/src/polar/util/buildinfo.cpp
)

# Polar Engine
add_library(polar STATIC ${POLAR_SRCS})
target_include_directories(polar PRIVATE ${POLAR_INCLUDE_DIRS})
target_link_directories(polar PRIVATE ${POLAR_LIBRARY_DIRS})
target_link_libraries(polar ${POLAR_LIBS})
set_property(TARGET polar PROPERTY CXX_STANDARD 17)
set_property(TARGET polar PROPERTY CXX_STANDARD_REQUIRED ON)
add_dependencies(polar invalidate_buildinfo)

set(ENABLE_CLANG_TIDY ON CACHE BOOL "Run clang-tidy on polar")
if(ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
	if(CLANG_TIDY_EXECUTABLE)
		message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXECUTABLE}")
		set(CLANG_TIDY_CHECKS "modernize*,performance*")
		set_property(TARGET polar PROPERTY CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_CURRENT_SOURCE_DIR}/include/polar'")
	endif()
endif()

# Asset Builder
add_executable(assetbuilder ${ASSETBUILDER_SRCS})
target_include_directories(assetbuilder PRIVATE ${POLAR_INCLUDE_DIRS})
target_link_libraries(assetbuilder ${ASSETBUILDER_LIBS})
set_property(TARGET assetbuilder PROPERTY CXX_STANDARD 17)
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
	set(POLAR_LIBRARY_DIRS ${POLAR_LIBRARY_DIRS} PARENT_SCOPE)
	set(POLAR_LIBS         ${POLAR_LIBS} PARENT_SCOPE)
	set(POLAR_DYLIBS       ${POLAR_DYLIBS} PARENT_SCOPE)
	set(POLAR_FRAMEWORKS   ${POLAR_FRAMEWORKS} PARENT_SCOPE)
endif()

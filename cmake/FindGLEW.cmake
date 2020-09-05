find_path(GLEW_INCLUDE_DIR "GL/glew.h" PATH_SUFFIXES include PATHS ${GLEW_ROOT_DIR})

if(GLEW_INCLUDE_DIR)
	set(GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR} CACHE STRING "Where the GLEW headers can be found")
endif()

if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
		find_library(GLEW_LIBRARY_TEMP NAMES "glew32" PATH_SUFFIXES lib/Release/x64 PATHS ${GLEW_ROOT_DIR})
		find_file(GLEW_DYLIB "glew32.dll" PATH_SUFFIXES bin/Release/x64 PATHS ${GLEW_ROOT_DIR})
	else()
		find_library(GLEW_LIBRARY_TEMP NAMES "glew32" PATH_SUFFIXES lib/Release/Win32 PATHS ${GLEW_ROOT_DIR})
		find_file(GLEW_DYLIB "glew32.dll" PATH_SUFFIXES bin/Release/Win32 PATHS ${GLEW_ROOT_DIR})
	endif()
endif()

if(GLEW_LIBRARY_TEMP)
	set(GLEW_LIBRARY ${GLEW_LIBRARY_TEMP} CACHE STRING "Where the GLEW library can be found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)

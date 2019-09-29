find_path(OpenVR_INCLUDE_DIR "openvr.h" PATH_SUFFIXES headers PATHS ${OpenVR_ROOT_DIR})

if(OpenVR_INCLUDE_DIR)
	set(OpenVR_INCLUDE_DIR ${OpenVR_INCLUDE_DIR} CACHE STRING "Where the OpenVR headers can be found")
endif()

if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
		find_library(OpenVR_LIBRARY_TEMP NAMES "openvr_api" "OpenVR" PATH_SUFFIXES lib/win64 PATHS ${OpenVR_ROOT_DIR})
		find_file(OpenVR_DYLIB "openvr_api.dll" PATH_SUFFIXES bin/win64 PATHS ${OpenVR_ROOT_DIR} NO_DEFAULT_PATH)
	else()
		find_library(OpenVR_LIBRARY_TEMP NAMES "openvr_api" "OpenVR" PATH_SUFFIXES lib/win32 PATHS ${OpenVR_ROOT_DIR})
		find_file(OpenVR_DYLIB "openvr_api.dll" PATH_SUFFIXES bin/win32 PATHS ${OpenVR_ROOT_DIR} NO_DEFAULT_PATH)
	endif()
else()
	find_library(OpenVR_LIBRARY_TEMP NAMES "openvr_api" "OpenVR" PATH_SUFFIXES lib/win32 lib/win64 lib/linux64 PATHS ${OpenVR_ROOT_DIR})
endif()

if(OpenVR_LIBRARY_TEMP)
	set(OpenVR_LIBRARY ${OpenVR_LIBRARY_TEMP} CACHE STRING "Where the OpenVR library can be found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenVR REQUIRED_VARS OpenVR_INCLUDE_DIR)

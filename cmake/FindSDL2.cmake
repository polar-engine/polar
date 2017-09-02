find_path(SDL2_INCLUDE_DIR "SDL.h" PATH_SUFFIXES include/SDL2 include PATHS ${CMAKE_SOURCE_DIR})
find_library(SDL2_LIBRARY_TEMP "SDL2" PATH_SUFFIXES lib/win32 lib/macos PATHS ${CMAKE_SOURCE_DIR})

if(SDL2_LIBRARY_TEMP)
	if(APPLE)
		set(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
	endif()

	set(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL2 library can be found")
	set(SDL2_LIBRARY_TEMP "${SDL2_LIBRARY_TEMP}" CACHE INTERNAL "")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR)

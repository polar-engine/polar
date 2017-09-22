find_path(SDL2_INCLUDE_DIR "SDL.h" PATH_SUFFIXES include/SDL2 include PATHS ${SDL2_ROOT_DIR})
find_library(SDL2_LIBRARY_TEMP "SDL2" PATH_SUFFIXES lib/x86 PATHS ${SDL2_ROOT_DIR})

if(WIN32)
	find_file(SDL2_DYLIB "SDL2.dll" PATH_SUFFIXES lib/x86 PATHS ${SDL2_ROOT_DIR})
endif()

if(SDL2_LIBRARY_TEMP)
	if(APPLE)
		set(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
	endif()

	set(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL2 library can be found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARY)

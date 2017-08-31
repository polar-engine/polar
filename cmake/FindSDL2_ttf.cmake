find_path(TTF_INCLUDE_DIR "SDL_ttf.h" PATH_SUFFIXES include/SDL2 include PATHS ${CMAKE_SOURCE_DIR})
find_library(TTF_LIBRARY_TEMP "SDL2_ttf" PATH_SUFFIXES lib/win32 lib PATHS ${CMAKE_SOURCE_DIR})

if(TTF_LIBRARY_TEMP)
	set(TTF_LIBRARY ${TTF_LIBRARY_TEMP} CACHE STRING "Where the ttf library can be found")
	set(TTF_LIBRARY_TEMP "${TTF_LIBRARY_TEMP}" CACHE INTERNAL "")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2_ttf REQUIRED_VARS TTF_LIBRARY TTF_INCLUDE_DIR)

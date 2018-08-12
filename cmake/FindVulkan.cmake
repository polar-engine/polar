find_path(VULKAN_INCLUDE_DIR "vulkan/vulkan.h" PATH_SUFFIXES include Include PATHS ${VULKAN_ROOT_DIR})
find_library(VULKAN_LIBRARY_TEMP NAMES "vulkan" "vulkan-1" PATH_SUFFIXES lib Lib32 PATHS ${VULKAN_ROOT_DIR})

if(VULKAN_INCLUDE_DIR)
	set(VULKAN_INCLUDE_DIR ${VULKAN_INCLUDE_DIR} CACHE STRING "Where the Vulkan header can be found")
endif()

if(APPLE)
	set(VULKAN_DYLIB ${VULKAN_LIBRARY_TEMP})
endif()

if(VULKAN_LIBRARY_TEMP)
	set(VULKAN_LIBRARY ${VULKAN_LIBRARY_TEMP} CACHE STRING "Where the Vulkan library can be found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan REQUIRED_VARS VULKAN_INCLUDE_DIR VULKAN_LIBRARY)

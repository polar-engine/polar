find_path(VULKAN_INCLUDE_DIR "vulkan/vulkan.h" PATH_SUFFIXES Include PATHS ${VULKAN_ROOT_DIR})

if(VULKAN_INCLUDE_DIR)
	set(VULKAN_INCLUDE_DIR ${VULKAN_INCLUDE_DIR} CACHE STRING "Where the Vulkan header can be found")
endif()

if(WIN32)
	find_library(VULKAN_LIBRARY_TEMP "vulkan-1" PATH_SUFFIXES Lib32 PATHS ${VULKAN_ROOT_DIR})
endif()

if(VULKAN_LIBRARY_TEMP)
	set(VULKAN_LIBRARY ${VULKAN_LIBRARY_TEMP} CACHE STRING "Where the Vulkan library can be found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan REQUIRED_VARS VULKAN_INCLUDE_DIR VULKAN_LIBRARY)

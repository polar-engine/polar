find_path(GLM_INCLUDE_DIR "glm/glm.hpp" PATHS ${GLM_ROOT_DIR})

if(GLM_INCLUDE_DIR)
	set(GLM_INCLUDE_DIR ${GLM_INCLUDE_DIR} CACHE STRING "Where the GLM headers can be found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM REQUIRED_VARS GLM_INCLUDE_DIR)

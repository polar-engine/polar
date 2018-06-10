#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define POLAR_VK_FNS \
	POLAR_VK_GLOBAL_FN(vkCreateInstance)                  \
	POLAR_VK_INSTANCE_FN(vkDestroyInstance)               \
	POLAR_VK_INSTANCE_FN(vkCreateDebugReportCallbackEXT)  \
	POLAR_VK_INSTANCE_FN(vkDestroyDebugReportCallbackEXT)

#define POLAR_VK_DEVICE_FN(NAME)   static PFN_##NAME NAME = nullptr;
#define POLAR_VK_GLOBAL_FN(NAME)   static PFN_##NAME NAME = nullptr;
#define POLAR_VK_INSTANCE_FN(NAME) static PFN_##NAME NAME = nullptr;
POLAR_VK_FNS
#undef POLAR_VK_INSTANCE_FN
#undef POLAR_VK_GLOBAL_FN
#undef POLAR_VK_DEVICE_FN

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;

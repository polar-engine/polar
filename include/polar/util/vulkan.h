#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define POLAR_VK_FNS \
	POLAR_VK_DEVICE_FN(vkCreateImageView)                           \
	POLAR_VK_DEVICE_FN(vkCreateSwapchainKHR)                        \
	POLAR_VK_DEVICE_FN(vkDestroyDevice)                             \
	POLAR_VK_DEVICE_FN(vkDestroyImageView)                          \
	POLAR_VK_DEVICE_FN(vkDestroySwapchainKHR)                       \
	POLAR_VK_DEVICE_FN(vkGetDeviceQueue)                            \
	POLAR_VK_DEVICE_FN(vkGetSwapchainImagesKHR)                     \
	POLAR_VK_GLOBAL_FN(vkCreateInstance)                            \
	POLAR_VK_INSTANCE_FN(vkCreateDevice)                            \
	POLAR_VK_INSTANCE_FN(vkDestroyInstance)                         \
	POLAR_VK_INSTANCE_FN(vkDestroySurfaceKHR)                       \
	POLAR_VK_INSTANCE_FN(vkEnumerateDeviceExtensionProperties)      \
	POLAR_VK_INSTANCE_FN(vkEnumeratePhysicalDevices)                \
	POLAR_VK_INSTANCE_FN(vkGetDeviceProcAddr)                       \
	POLAR_VK_INSTANCE_FN(vkGetPhysicalDeviceProperties)             \
	POLAR_VK_INSTANCE_FN(vkGetPhysicalDeviceQueueFamilyProperties)  \
	POLAR_VK_INSTANCE_FN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
	POLAR_VK_INSTANCE_FN(vkGetPhysicalDeviceSurfaceFormatsKHR)      \
	POLAR_VK_INSTANCE_FN(vkGetPhysicalDeviceSurfacePresentModesKHR) \
	POLAR_VK_INSTANCE_FN(vkGetPhysicalDeviceSurfaceSupportKHR)

#define POLAR_VK_DEBUG_FNS \
	POLAR_VK_INSTANCE_FN(vkCreateDebugReportCallbackEXT)            \
	POLAR_VK_INSTANCE_FN(vkDestroyDebugReportCallbackEXT)           \

#define POLAR_VK_DEVICE_FN(NAME)   static PFN_##NAME NAME = nullptr;
#define POLAR_VK_GLOBAL_FN(NAME)   static PFN_##NAME NAME = nullptr;
#define POLAR_VK_INSTANCE_FN(NAME) static PFN_##NAME NAME = nullptr;
POLAR_VK_FNS
POLAR_VK_DEBUG_FNS
#undef POLAR_VK_INSTANCE_FN
#undef POLAR_VK_GLOBAL_FN
#undef POLAR_VK_DEVICE_FN

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;

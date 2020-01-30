#pragma once

#include <map>
#include <vector>
#include <polar/support/action/controller.h>
#include <polar/support/action/keyboard.h>
#include <polar/support/action/mouse.h>
#include <polar/system/renderer/base.h>
#include <polar/util/sdl.h>
#include <polar/util/vulkan.h>
#include <SDL_vulkan.h>

namespace polar::system::renderer {
	class vulkan : public base {
	  private:
		struct queue_family_indices_t {
			int graphicsFamily = -1;
			int presentFamily = -1;

			inline bool complete() {
				return graphicsFamily >= 0 &&
				       presentFamily >= 0;
			}
		};

		struct swapchain_support_details_t {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		bool fullscreen = false;

		SDL_Window *window = nullptr;
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		queue_family_indices_t indices;
		VkDevice logicalDevice = VK_NULL_HANDLE;
		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> swapchainImages;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;
		std::vector<VkImageView> swapchainImageViews;

		const std::vector<const char *> validationLayers = {
			"VK_LAYER_LUNARG_standard_validation"
		};

		const std::vector<const char *> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const bool enableValidationLayers =
#ifdef _DEBUG
		true
#else
		false
#endif
		;

		void init_global_fns() {
			SDL(vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr());
#define POLAR_VK_DEVICE_FN(NAME)
#define POLAR_VK_GLOBAL_FN(NAME)                                                 \
			log()->debug("Vulkan: loading global function: ", #NAME);            \
			NAME = (PFN_##NAME)vkGetInstanceProcAddr(VK_NULL_HANDLE, #NAME);     \
			if(!NAME) {                                                          \
				log()->fatal("Vulkan: failed to load global function: ", #NAME); \
			}                                                                    \
			log()->debug("Vulkan: loaded global function: ", #NAME);
#define POLAR_VK_INSTANCE_FN(NAME)
			POLAR_VK_FNS
#undef POLAR_VK_INSTANCE_FN
#undef POLAR_VK_GLOBAL_FN
#undef POLAR_VK_DEVICE_FN
		}

		void init_instance() {
			if(!SDL(window = SDL_CreateWindow(
			            "Polar Engine", SDL_WINDOWPOS_CENTERED,
			            SDL_WINDOWPOS_CENTERED, width, height,
			            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN |
			                SDL_WINDOW_RESIZABLE |
			                (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)))) {
				log()->fatal("failed to create window");
			}

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Polar Engine";
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.pEngineName = "Polar Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			unsigned int count = 0;
			SDL(SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr));
			std::vector<const char *> requiredExtensions(count);
			SDL(SDL_Vulkan_GetInstanceExtensions(window, &count, requiredExtensions.data()));

			if(enableValidationLayers) {
				requiredExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}

			log()->debug("Vulkan: required extensions:");
			for(auto &ext : requiredExtensions) {
				log()->debug("* ", ext);
			}

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = requiredExtensions.size();
			createInfo.ppEnabledExtensionNames = requiredExtensions.data();

			if(enableValidationLayers) {
				log()->debug("Vulkan: required layers:");
				for(auto &layer : validationLayers) {
					log()->debug("* ", layer);
				}

				createInfo.enabledLayerCount = validationLayers.size();
				createInfo.ppEnabledLayerNames = validationLayers.data();
			} else {
				createInfo.enabledLayerCount = 0;
			}

			VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
			if(result != VK_SUCCESS) {
				log()->fatal("Vulkan: failed to create instance (VkResult = ", result, ')');
			}
			log()->verbose("Vulkan: created instance");
		}

		void init_instance_fns() {
#define POLAR_VK_DEVICE_FN(NAME)
#define POLAR_VK_GLOBAL_FN(NAME)
#define POLAR_VK_INSTANCE_FN(NAME) \
			log()->debug("Vulkan: loading instance function: ", #NAME);            \
			NAME = (PFN_##NAME)vkGetInstanceProcAddr(instance, #NAME);             \
			if(!NAME) {                                                            \
				log()->fatal("Vulkan: failed to load instance function: ", #NAME); \
			}                                                                      \
			log()->debug("Vulkan: loaded instance function: ", #NAME);

			POLAR_VK_FNS

			if(enableValidationLayers) {
				POLAR_VK_DEBUG_FNS
			}
#undef POLAR_VK_INSTANCE_FN
#undef POLAR_VK_GLOBAL_FN
#undef POLAR_VK_DEVICE_FN
		}

		void init_debug_callback() {
			if(enableValidationLayers) {
				VkDebugReportCallbackCreateInfoEXT createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
				createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
				                   VK_DEBUG_REPORT_WARNING_BIT_EXT |
				                   VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
				                   VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
				                   VK_DEBUG_REPORT_DEBUG_BIT_EXT;
				createInfo.pfnCallback = debug_callback;

				VkResult result = vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &debugCallback);
				if(result != VK_SUCCESS) {
					log()->fatal("Vulkan: failed to create debug report callback");
				}
				log()->verbose("Vulkan: created debug report callback");
			}
		}

		void init_surface() {
			if(!SDL(SDL_Vulkan_CreateSurface(window, instance, &surface))) {
				log()->fatal("Vulkan: failed to create SDL surface");
			}
			log()->verbose("Vulkan: created SDL surface");
		}

		void init_physical_device() {
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
			if(deviceCount == 0) {
				log()->fatal("Vulkan: failed to find device");
			}
			log()->verbose("Vulkan: found ", deviceCount, " device", deviceCount == 1 ? "" : "s");

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			std::multimap<int, VkPhysicalDevice> candidates;

			for(const auto &device : devices) {
				candidates.emplace(device_score(device), device);
			}

			auto it = candidates.rbegin();
			if(it->first <= 0) {
				log()->fatal("Vulkan: failed to find suitable device");
			}
			log()->verbose("Vulkan: found suitable device");

			physicalDevice = it->second;
			indices = find_queue_families(physicalDevice);
		}

		void init_logical_device() {
			std::unordered_set<int> uniqueQueueFamilies = {
				indices.graphicsFamily,
				indices.presentFamily
			};

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

			float priority = 1.0f;
			for(int queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &priority;
				queueCreateInfos.emplace_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures = {};

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.queueCreateInfoCount = queueCreateInfos.size();
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = deviceExtensions.size();
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if(enableValidationLayers) {
				createInfo.enabledLayerCount = validationLayers.size();
				createInfo.ppEnabledLayerNames = validationLayers.data();
			} else {
				createInfo.enabledLayerCount = 0;
			}

			VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
			if(result != VK_SUCCESS) {
				log()->fatal("Vulkan: failed to create logical device");
			}
			log()->verbose("Vulkan: created logical device");
		}

		void init_device_fns() {
#define POLAR_VK_DEVICE_FN(NAME)                                                 \
			log()->debug("Vulkan: loading device function: ", #NAME);            \
			NAME = (PFN_##NAME)vkGetDeviceProcAddr(logicalDevice, #NAME);        \
			if(!NAME) {                                                          \
				log()->fatal("Vulkan: failed to load device function: ", #NAME); \
			}                                                                    \
			log()->debug("Vulkan: loaded device function: ", #NAME);
#define POLAR_VK_GLOBAL_FN(NAME)
#define POLAR_VK_INSTANCE_FN(NAME)
			POLAR_VK_FNS
#undef POLAR_VK_INSTANCE_FN
#undef POLAR_VK_GLOBAL_FN
#undef POLAR_VK_DEVICE_FN
		}

		void init_swapchain() {
			auto swapchain_support = query_swapchain_support(physicalDevice);
			auto format = choose_swap_surface_format(swapchain_support.formats);
			auto presentMode = choose_swap_present_mode(swapchain_support.presentModes);
			auto extent = choose_swap_extent(swapchain_support.capabilities);

			uint32_t imageCount = swapchain_support.capabilities.minImageCount + 1;
			if(swapchain_support.capabilities.maxImageCount > 0 &&
			   imageCount > swapchain_support.capabilities.maxImageCount) {
				imageCount = swapchain_support.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = format.format;
			createInfo.imageColorSpace = format.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			auto indices = find_queue_families(physicalDevice);
			if(indices.graphicsFamily != indices.presentFamily) {
				uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			} else {
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}

			createInfo.preTransform = swapchain_support.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain);
			if(result != VK_SUCCESS) {
				log()->fatal("Vulkan: failed to create swapchain");
			}
			log()->verbose("Vulkan: created swapchain");

			vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
			swapchainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());

			swapchainImageFormat = format.format;
			swapchainExtent = extent;
		}

		void init_image_views() {
			swapchainImageViews.resize(swapchainImages.size());

			for(size_t i = 0; i < swapchainImages.size(); ++i) {
				VkImageViewCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapchainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapchainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				VkResult result = vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapchainImageViews[i]);
				if(result != VK_SUCCESS) {
					log()->fatal("Vulkan: failed to create image view");
				}
				log()->verbose("Vulkan: created image view");
			}
		}

		void init_pipeline() {
		}

		int device_score(const VkPhysicalDevice &device) {
			int score = 0;

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);

			if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				score += 1000;
			}
			score += props.limits.maxImageDimension2D;

			auto indices = find_queue_families(device);

			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for(const auto &extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}
			bool extensionsSupported = requiredExtensions.empty();

			bool swapchainAdequate = false;
			if(extensionsSupported) {
				auto swapchain_support = query_swapchain_support(device);
				swapchainAdequate = !swapchain_support.formats.empty() &&
				                    !swapchain_support.presentModes.empty();
			}

			if(!indices.complete() || !extensionsSupported || !swapchainAdequate) {
				score = 0;
			}

			return score;
		}

		queue_family_indices_t find_queue_families(const VkPhysicalDevice &device) {
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			queue_family_indices_t indices;

			int i = 0;
			for(const auto &queueFamily : queueFamilies) {
				if(queueFamily.queueCount > 0) {
					if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						indices.graphicsFamily = i;
					}

					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
					if(presentSupport) {
						indices.presentFamily = i;
					}
				}
				if(indices.complete()) { break; }
				++i;
			}

			return indices;
		}

		swapchain_support_details_t query_swapchain_support(const VkPhysicalDevice &device) {
			swapchain_support_details_t details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			if(formatCount > 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
			if(presentModeCount > 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &formats) {
			if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
				return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
			}
			for(const auto &format : formats) {
				if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return format;
				}
			}
			return formats[0];
		}

		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> presentModes) {
			VkPresentModeKHR bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			for(const auto &presentMode : presentModes) {
				switch(presentMode) {
				case VK_PRESENT_MODE_MAILBOX_KHR:
					return presentMode;
				case VK_PRESENT_MODE_FIFO_KHR:
					bestMode = presentMode;
					break;
				}
			}
			return bestMode;
		}

		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
			if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
				return capabilities.currentExtent;
			}
			VkExtent2D extent = {};
			extent.width  = std::max(capabilities.minImageExtent.width,  std::min(capabilities.maxImageExtent.width,  (uint32_t)width));
			extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height));
			return extent;
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL
		debug_callback(VkDebugReportFlagsEXT flags,
		               VkDebugReportObjectTypeEXT objType,
		               uint64_t obj, size_t location, int32_t code,
		               const char *layerPrefix, const char *msg,
		               void *userData) {
			if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
				log()->fatal("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
				log()->warning("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
				log()->warning("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
				log()->debug("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
				log()->debug("Vulkan: DEBUG validation layer: ", msg);
			}
			return VK_FALSE;
		}

		void handleSDL(SDL_Event &ev) {
			namespace kb         = support::action::keyboard;
			namespace mouse      = support::action::mouse;
			namespace controller = support::action::controller;

			support::input::key key;

			auto act = engine->get<action>().lock();

			switch(ev.type) {
			case SDL_QUIT:
				engine->quit();
				break;
			case SDL_WINDOWEVENT:
				break;
			case SDL_KEYDOWN:
				if(ev.key.repeat == 0) {
					key = mkKeyFromSDL(ev.key.keysym.sym);
					if(act) {
						act->trigger_digital(kb::key_ti(key), true);
					}
				}
				break;
			case SDL_KEYUP:
				key = mkKeyFromSDL(ev.key.keysym.sym);
				if(act) {
					act->trigger_digital(kb::key_ti(key), false);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				key = mkMouseButtonFromSDL(ev.button.button);
				if(act) {
					act->trigger_digital(kb::key_ti(key), true);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				key = mkMouseButtonFromSDL(ev.button.button);
				if(act) {
					act->trigger_digital(kb::key_ti(key), false);
				}
				break;
			case SDL_MOUSEMOTION:
				if(act) {
					// XXX: this is hacky
					act->accumulate<mouse::position_x>(Decimal(ev.motion.x));
					act->accumulate<mouse::position_y>(Decimal(ev.motion.y));

					act->accumulate<mouse::motion_x>(Decimal(ev.motion.xrel));
					act->accumulate<mouse::motion_y>(Decimal(ev.motion.yrel));
				}
				break;
			case SDL_MOUSEWHEEL:
				if(act) {
					act->accumulate<mouse::wheel_x>(Decimal(ev.wheel.x));
					act->accumulate<mouse::wheel_y>(Decimal(ev.wheel.y));
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				key = mkButtonFromSDL(
					static_cast<SDL_GameControllerButton>(ev.cbutton.button));
				if(act) {
					act->trigger_digital(kb::key_ti(key), true);
				}
				break;
			case SDL_CONTROLLERBUTTONUP:
				key = mkButtonFromSDL(
					static_cast<SDL_GameControllerButton>(ev.cbutton.button));
				if(act) {
					act->trigger_digital(kb::key_ti(key), false);
				}
				break;
			case SDL_CONTROLLERAXISMOTION:
				switch(ev.caxis.axis) {
				case 0: // x axis
					if(act) {
						act->accumulate<controller::motion_x>(ev.caxis.value);
					}
					break;
				case 1: // y axis
					if(act) {
						act->accumulate<controller::motion_y>(ev.caxis.value);
					}
					break;
				}
				break;
			}
		}
	  protected:
		void init() override {
			if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) {
				log()->fatal("failed to init SDL");
			}
			if(!SDL(TTF_Init())) { log()->fatal("failed to init TTF"); }
			SDL(SDL_Vulkan_LoadLibrary(NULL));

			init_global_fns();
			init_instance();
			init_instance_fns();
			init_debug_callback();
			init_surface();
			init_physical_device();
			init_logical_device();
			init_device_fns();
			init_swapchain();
			init_image_views();
			init_pipeline();

			vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
			vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);
		}

		void update(DeltaTicks &) override {
			// handle input at beginning of frame to reduce delays in other systems
			SDL_Event event;
			while(SDL_PollEvent(&event)) { handleSDL(event); }
			SDL_ClearError();
		}

		void makepipeline(const std::vector<std::string> &) override {}
	  public:
		vulkan(core::polar *engine) : base(engine) {}

		~vulkan() {
			for(auto &view : swapchainImageViews) {
				vkDestroyImageView(logicalDevice, view, nullptr);
			}
			vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
			vkDestroyDevice(logicalDevice, nullptr);
			vkDestroySurfaceKHR(instance, surface, nullptr);
			if(enableValidationLayers) {
				vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
			}
			vkDestroyInstance(instance, nullptr);
			SDL(SDL_DestroyWindow(window));
			SDL(SDL_Vulkan_UnloadLibrary());
			SDL(SDL_Quit());
		}

		static bool supported() { return true; }

		void setmousecapture(bool capture) override {}
		void setfullscreen(bool fullscreen) override {}
		void setdepthtest(bool depthtest) override {}
		void setpipeline(const std::vector<std::string> &names) override {}
		void setclearcolor(const Point4 &color) override {}

		void resize(uint16_t w, uint16_t h) override {
			width = w;
			height = h;

			SDL(SDL_SetWindowSize(window, width, height));
		}

		Decimal getuniform_decimal(const std::string &name,
		                           const Decimal def) override { return 0; }
		Point3 getuniform_point3(const std::string &name,
		                         const Point3 def) override { return Point3(0); }

		void setuniform(const std::string &name, glm::uint32 x,
		                bool force = false) override {}
		void setuniform(const std::string &name, Decimal x,
		                bool force = false) override {}
		void setuniform(const std::string &name, Point3 p,
		                bool force = false) override {}
	};
} // namespace polar::system::renderer

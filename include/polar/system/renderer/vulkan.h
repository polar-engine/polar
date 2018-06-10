#pragma once

#include <vector>
#include <polar/system/renderer/base.h>
#include <polar/util/sdl.h>
#include <polar/util/vulkan.h>
#include <SDL_vulkan.h>

namespace polar::system::renderer {
	class vulkan : public base {
	  private:
		bool fullscreen = false;

		SDL_Window *window = nullptr;
		VkInstance instance;
		VkDebugReportCallbackEXT debugCallback;

		const std::vector<const char *> validationLayers = {
			"VK_LAYER_LUNARG_standard_validation"
		};

		const bool enableValidationLayers =
#ifdef _DEBUG
		true
#else
		false
#endif
		;

		void init() override {
			if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) {
				debugmanager()->fatal("failed to init SDL");
			}
			if(!SDL(TTF_Init())) { debugmanager()->fatal("failed to init TTF"); }
			SDL(SDL_Vulkan_LoadLibrary(NULL));

			init_global_fns();
			init_instance();
			init_instance_fns();
			init_debug_callback();
		}

		void update(DeltaTicks &) override {}

		void init_global_fns() {
			vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
#define POLAR_VK_DEVICE_FN(NAME)
#define POLAR_VK_GLOBAL_FN(NAME)                                                          \
			NAME = (PFN_##NAME)vkGetInstanceProcAddr(VK_NULL_HANDLE, #NAME);              \
			if(!NAME) {                                                                   \
				debugmanager()->fatal("Vulkan: failed to load global function: ", #NAME); \
			}                                                                             \
			debugmanager()->debug("Vulkan: loaded global function: ", #NAME);
#define POLAR_VK_INSTANCE_FN(NAME)
			POLAR_VK_FNS
#undef POLAR_VK_INSTANCE_FN
#undef POLAR_VK_GLOBAL_FN
#undef POLAR_VK_DEVICE_FN
		}

		void init_instance() {
			SDL_Window *window = nullptr;
			if(!SDL(window = SDL_CreateWindow(
			            "Polar Engine", SDL_WINDOWPOS_CENTERED,
			            SDL_WINDOWPOS_CENTERED, width, height,
			            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN |
			                SDL_WINDOW_RESIZABLE |
			                (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)))) {
				debugmanager()->fatal("failed to create window");
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

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = requiredExtensions.size();
			createInfo.ppEnabledExtensionNames = requiredExtensions.data();

			if(enableValidationLayers) {
				createInfo.enabledLayerCount = validationLayers.size();
				createInfo.ppEnabledLayerNames = validationLayers.data();
			} else {
				createInfo.enabledLayerCount = 0;
			}

			VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
			if(result != VK_SUCCESS) {
				debugmanager()->fatal("Vulkan: failed to create instance");
			}
			debugmanager()->debug("Vulkan: created instance");
		}

		void init_instance_fns() {
#define POLAR_VK_DEVICE_FN(NAME)
#define POLAR_VK_GLOBAL_FN(NAME)
#define POLAR_VK_INSTANCE_FN(NAME)                                                          \
			NAME = (PFN_##NAME)vkGetInstanceProcAddr(instance, #NAME);                      \
			if(!NAME) {                                                                     \
				debugmanager()->fatal("Vulkan: failed to load instance function: ", #NAME); \
			}                                                                               \
			debugmanager()->debug("Vulkan: loaded instance function: ", #NAME);
			POLAR_VK_FNS
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
					debugmanager()->fatal("Vulkan: failed to create debug report callback");
				}
				debugmanager()->debug("Vulkan: created debug report callback");
			}
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL
		debug_callback(VkDebugReportFlagsEXT flags,
		               VkDebugReportObjectTypeEXT objType,
		               uint64_t obj, size_t location, int32_t code,
		               const char *layerPrefix, const char *msg,
		               void *userData) {
			if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
				debugmanager()->fatal("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
				debugmanager()->warning("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
				debugmanager()->warning("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
				debugmanager()->verbose("Vulkan: validation layer: ", msg);
			} else if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
				debugmanager()->debug("Vulkan: validation layer: ", msg);
			}
			return VK_FALSE;
		}

		void makepipeline(const std::vector<std::string> &) override {}
	  public:
		vulkan(core::polar *engine) : base(engine) {}

		~vulkan() {
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
		void setpipeline(const std::vector<std::string> &names) override {}
		void setclearcolor(const Point4 &color) override {}

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

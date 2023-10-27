#include "renderer.h"
#include "core.h"
#include "log.h"

namespace gp {

#ifndef ENABLE_VALIDATION_LAYERS
#define ENABLE_VALIDATION_LAYERS false
#elif ENABLE_VALIDATION_LAYERS
// TODO: custom callback for printing validation layer messages
static const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
#endif

// clang-format off
static bool create_instance(SDL_Window *window, VkInstance &instance);
static bool create_surface(SDL_Window *window, VkInstance &instance, VkSurfaceKHR &surface);
static bool create_physical_device(VkInstance &instance, VkPhysicalDevice &physical_device);

static void print_available_extensions();
static void print_available_layers();
static bool check_device_suitability(VkPhysicalDevice &device);
// clang-format on

bool Renderer::init(SDL_Window *window) {
	data.window = window;

	print_available_extensions();
	print_available_layers();

	if (!create_instance(data.window, data.instance)) {
		log::error("Failed to create Vulkan instance");
		return false;
	}

	if (!create_surface(data.window, data.instance, data.surface)) {
		log::error("Failed to create Vulkan instance");
		return false;
	}

	if (!create_physical_device(data.instance, data.physical_device)) {
		log::error("Failed to pick physical device");
		return false;
	}

	return true;
}

void Renderer::draw() {}

void Renderer::cleanup() {
	vkDestroySurfaceKHR(data.instance, data.surface, nullptr);
	vkDestroyInstance(data.instance, nullptr);
}

static bool create_instance(SDL_Window *window, VkInstance &instance) {
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Graphics Playground";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.apiVersion = VK_API_VERSION_1_3;

	u32 ext_count;
	SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr);

	const char *ext_names[ext_count];
	SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names);

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = ext_count;
	create_info.ppEnabledExtensionNames = ext_names;

#if ENABLE_VALIDATION_LAYERS
	create_info.enabledLayerCount = sizeof(validation_layers) / sizeof(char *);
	create_info.ppEnabledLayerNames = validation_layers;
#else
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = nullptr;
#endif

	return vkCreateInstance(&create_info, nullptr, &instance) == VK_SUCCESS;
}

static bool create_surface(SDL_Window *window, VkInstance &instance,
						   VkSurfaceKHR &surface) {
	return SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_TRUE;
}

static bool create_physical_device(VkInstance &instance,
								   VkPhysicalDevice &physical_device) {
	physical_device = VK_NULL_HANDLE;

	u32 device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

	if (device_count == 0) {
		log::error("Failed to find a GPU with Vulkan support");
		return false;
	}

	VkPhysicalDevice devices[device_count];
	vkEnumeratePhysicalDevices(instance, &device_count, devices);

	// TODO: probably want some logic to prefer discrete GPU if available
	for (auto &device : devices) {
		if (check_device_suitability(device)) {
			physical_device = device;
			break;
		}
	}

	if (physical_device == VK_NULL_HANDLE) {
		log::error("Failed to find a suitable GPU");
		return false;
	}

	return true;
}

static void print_available_extensions() {
	u32 ext_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

	VkExtensionProperties ext_props[ext_count];
	vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, ext_props);

	log::debug("Available Vulkan Extensions:");
	for (auto &ext : ext_props)
		log::debug(ext.extensionName);
}

static void print_available_layers() {
	u32 layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	VkLayerProperties layer_props[layer_count];
	vkEnumerateInstanceLayerProperties(&layer_count, layer_props);

	log::debug("Available Vulkan Layers:");
	for (auto &layer : layer_props)
		log::debug(layer.layerName);
}

static bool check_device_suitability(VkPhysicalDevice &device) {
	VkPhysicalDeviceProperties device_props;
	VkPhysicalDeviceFeatures device_feats;

	vkGetPhysicalDeviceProperties(device, &device_props);
	vkGetPhysicalDeviceFeatures(device, &device_feats);

	log::debug(std::format("Found GPU: {}", device_props.deviceName));

	// TODO: more advanced feature checks?
	switch (device_props.deviceType) {
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		log::debug("Type: Discrete");
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		log::debug("Type: Integrated");
	default:
		log::debug("Type: Unknown");
	}

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	VkQueueFamilyProperties queue_family_props[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_props);

	log::debug(std::format("Available Queue Families: {}", queue_family_count));

	return false;
};

} // namespace gp

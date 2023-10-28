#include "renderer.h"
#include "core.h"
#include "log.h"
#include <optional>
#include <vulkan/vulkan_core.h>

namespace gp {

#ifndef ENABLE_VALIDATION_LAYERS
#define ENABLE_VALIDATION_LAYERS false
#elif ENABLE_VALIDATION_LAYERS
// TODO: custom callback for printing validation layer messages
static const char *enabled_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32 enabled_layer_count = sizeof(enabled_layers) / sizeof(char *);
#endif

static const char *enabled_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const u32 enabled_extension_count = sizeof(enabled_extensions) / sizeof(char *);

struct QueueFamilyIndices {
    std::optional<u32> graphics;
};

static bool create_instance(SDL_Window *window, VkInstance &instance);
static bool create_surface(SDL_Window *window, VkInstance &instance, VkSurfaceKHR &surface);
static bool select_physical_device(VkInstance &instance, VkPhysicalDevice &physical_device);
static bool create_logical_device(VkPhysicalDevice &physical_device, VkDevice &device, VkQueue &graphics_queue, VkQueue &present_queue);

static void print_available_extensions();
static void print_available_layers();
static bool check_device_suitability(VkPhysicalDevice &physical_device);
static QueueFamilyIndices get_queue_family_indices(VkPhysicalDevice &physical_device);
static bool check_extension_support(VkPhysicalDevice &physical_device);

bool
Renderer::init(SDL_Window *window)
{
    data.window = window;

    print_available_extensions();
    print_available_layers();

    if (!create_instance(data.window, data.instance)) {
        log::error("Failed to create Vulkan instance");
        return false;
    }

    if (!create_surface(data.window, data.instance, data.surface)) {
        log::error("Failed to create surface");
        return false;
    }

    if (!select_physical_device(data.instance, data.physical_device)) {
        log::error("Failed to select physical device");
        return false;
    }

    if (!create_logical_device(data.physical_device, data.device, data.graphics_queue, data.present_queue)) {
        log::error("Failed to create logical device");
        return false;
    }

    return true;
}

void
Renderer::draw()
{
}

void
Renderer::cleanup()
{
    vkDestroyDevice(data.device, nullptr);
    vkDestroySurfaceKHR(data.instance, data.surface, nullptr);
    vkDestroyInstance(data.instance, nullptr);
}

static bool
create_instance(SDL_Window *window,
                VkInstance &instance)
{
    VkApplicationInfo app_info = {};
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

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = ext_count;
    instance_create_info.ppEnabledExtensionNames = ext_names;

#if ENABLE_VALIDATION_LAYERS
    instance_create_info.enabledLayerCount = enabled_layer_count;
    instance_create_info.ppEnabledLayerNames = enabled_layers;
#else
    instance_create_info.enabledLayerCount = 0;
    instance_create_info.ppEnabledLayerNames = nullptr;
#endif

    VkResult result = vkCreateInstance(&instance_create_info,
                                       nullptr,
                                       &instance);

    return result == VK_SUCCESS;
}

static bool
select_physical_device(VkInstance &instance,
                       VkPhysicalDevice &physical_device)
{
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

static bool
create_logical_device(VkPhysicalDevice &physical_device,
                      VkDevice &device,
                      VkQueue &graphics_queue,
                      VkQueue &present_queue)
{
    // the selected gpu should always support graphics queue, if we get here
    QueueFamilyIndices queue_family_indices = get_queue_family_indices(physical_device);
    f32 queue_priorities = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_indices.graphics.value();
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priorities;

    VkPhysicalDeviceFeatures physical_device_feats = {};

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = &physical_device_feats;
    device_create_info.enabledExtensionCount = enabled_extension_count;
    device_create_info.ppEnabledExtensionNames = enabled_extensions;

#if ENABLE_VALIDATION_LAYERS
    device_create_info.enabledLayerCount = enabled_layer_count;
    device_create_info.ppEnabledLayerNames = enabled_layers;
#else
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
#endif

    VkResult result = vkCreateDevice(physical_device,
                                     &device_create_info,
                                     nullptr,
                                     &device);

    if (result != VK_SUCCESS) {
        return false;
    }

    vkGetDeviceQueue(device,
                     queue_family_indices.graphics.value(),
                     0,
                     &graphics_queue);

    // TODO: assuming graphics and present queues are identical for now
    present_queue = graphics_queue;

    return true;
}

static bool
create_surface(SDL_Window *window,
               VkInstance &instance,
               VkSurfaceKHR &surface)
{
    SDL_bool result = SDL_Vulkan_CreateSurface(window, instance, &surface);
    return result == SDL_TRUE;
}

static void
print_available_extensions()
{
    u32 ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

    VkExtensionProperties ext_props[ext_count];
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, ext_props);

    log::debug("Available Vulkan Extensions:");
    for (auto &ext : ext_props) {
        log::debug(ext.extensionName);
    }
}

static void
print_available_layers()
{
    u32 layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    VkLayerProperties layer_props[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, layer_props);

    log::debug("Available Vulkan Layers:");
    for (auto &layer : layer_props) {
        log::debug(layer.layerName);
    }
}

static bool
check_device_suitability(VkPhysicalDevice &physical_device)
{
    VkPhysicalDeviceProperties physical_device_props;
    VkPhysicalDeviceFeatures physical_device_feats;

    vkGetPhysicalDeviceProperties(physical_device, &physical_device_props);
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_feats);

    log::debug(std::format("Found GPU: {}", physical_device_props.deviceName));

    // TODO: more advanced feature checks?
    switch (physical_device_props.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        log::debug("Type: discrete");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        log::debug("Type: integrated");
        break;
    default:
        log::debug("Type: unknown");
        break;
    }

    bool graphics_queue_support = get_queue_family_indices(physical_device).graphics.has_value();
    bool extension_support = check_extension_support(physical_device);

    return graphics_queue_support && extension_support;
};

static QueueFamilyIndices
get_queue_family_indices(VkPhysicalDevice &physical_device)
{
    QueueFamilyIndices queue_family_indices = {};

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count,
                                             nullptr);

    VkQueueFamilyProperties queue_family_props[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count,
                                             queue_family_props);

    for (int i = 0; i < queue_family_count; ++i) {
        if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_indices.graphics = i;
        }
    }

    return queue_family_indices;
}

static bool
check_extension_support(VkPhysicalDevice &physical_device)
{
    u32 ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

    VkExtensionProperties ext_props[ext_count];
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, ext_props);

    bool ext_support[enabled_extension_count] = {false};

    for (u32 i = 0; i < enabled_extension_count; ++i) {
        for (auto &ext : ext_props) {
            if (strcmp(ext.extensionName, enabled_extensions[i])) {
                ext_support[i] = true;
            }
        }
    }

    for (u32 i = 0; i < enabled_extension_count; ++i) {
        if (!ext_support[i]) {
            log::error(std::format("Missing support for required extension: {}", enabled_extensions[i]));
            return false;
        }
    }

    return true;
}

} // namespace gp

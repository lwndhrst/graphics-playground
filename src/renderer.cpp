#include "renderer.h"

#include "core.h"
#include "log.h"

namespace gp {

#define CLAMP(val, hi, lo) (val > hi ? hi : (val < lo ? lo : val))

#ifndef ENABLE_VALIDATION_LAYERS
#define ENABLE_VALIDATION_LAYERS false
#elif ENABLE_VALIDATION_LAYERS
// TODO: custom callback for printing validation layer messages
static const char *enabled_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32 enabled_layer_count = sizeof(enabled_layers) / sizeof(usize);
#endif

static const char *enabled_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const u32 enabled_extension_count = sizeof(enabled_extensions) / sizeof(usize);

static bool
create_instance(
    SDL_Window *window,
    VkInstance &instance);

static bool
create_surface(
    SDL_Window *window,
    VkInstance &instance,
    VkSurfaceKHR &surface);

static bool
select_physical_device(
    VkInstance &instance,
    VkSurfaceKHR &surface,
    VkPhysicalDevice &physical_device,
    DeviceQueueFamilyIndices &queue_family_indices);

static bool
create_logical_device(
    VkPhysicalDevice &physical_device,
    VkDevice &device,
    DeviceQueueFamilyIndices &queue_family_indices,
    DeviceQueues &queues);

static bool
create_swapchain(
    SDL_Window *window,
    VkSurfaceKHR &surface,
    VkPhysicalDevice &physical_device);

static void
print_available_extensions();

static void
print_available_layers();

static bool
check_device_suitability(
    VkPhysicalDevice &physical_device,
    VkSurfaceKHR &surface);

static DeviceQueueFamilyIndices
get_queue_family_indices(
    VkPhysicalDevice &physical_device);

static bool
check_extension_support(
    VkPhysicalDevice &physical_device);

static DeviceSwapchainSupportDetails
get_swapchain_support_details(
    VkSurfaceKHR &surface,
    VkPhysicalDevice &physical_device);

static VkSurfaceFormatKHR
select_surface_format(
    u32 available_surface_format_count,
    VkSurfaceFormatKHR *available_surface_formats);

static VkPresentModeKHR
select_present_mode(
    u32 available_present_mode_count,
    VkPresentModeKHR *available_present_modes);

static VkExtent2D
select_swap_extent(
    SDL_Window *window,
    VkSurfaceCapabilitiesKHR &surface_capabilities);

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

    if (!select_physical_device(data.instance, data.surface, data.physical_device, data.queue_family_indices)) {
        log::error("Failed to select physical device");
        return false;
    }

    if (!create_logical_device(data.physical_device, data.device, data.queue_family_indices, data.queues)) {
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

    u32 extension_count;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);

    const char *extension_names[extension_count];
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names);

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = extension_count;
    instance_create_info.ppEnabledExtensionNames = extension_names;

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
create_surface(SDL_Window *window,
               VkInstance &instance,
               VkSurfaceKHR &surface)
{
    SDL_bool result = SDL_Vulkan_CreateSurface(window, instance, &surface);
    return result == SDL_TRUE;
}

static bool
select_physical_device(VkInstance &instance,
                       VkSurfaceKHR &surface,
                       VkPhysicalDevice &physical_device,
                       DeviceQueueFamilyIndices &queue_family_indices)
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
        if (check_device_suitability(device, surface)) {
            physical_device = device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        log::error("Failed to find a suitable GPU");
        return false;
    }

    queue_family_indices = get_queue_family_indices(physical_device);

    return true;
}

static bool
create_logical_device(VkPhysicalDevice &physical_device,
                      VkDevice &device,
                      DeviceQueueFamilyIndices &queue_family_indices,
                      DeviceQueues &queues)
{
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
                     &queues.graphics);

    // TODO: assuming graphics and present queues are identical for now
    queues.present = queues.graphics;

    return true;
}

static bool
create_swapchain(SDL_Window *window,
                 VkSurfaceKHR &surface,
                 VkPhysicalDevice &physical_device)
{

    return true;
}

static void
print_available_extensions()
{
    u32 extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    VkExtensionProperties extension_props[extension_count];
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_props);

    log::debug("Available Vulkan Extensions:");
    for (auto &extension : extension_props) {
        log::debug(extension.extensionName);
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

// TODO: more advanced feature checks?
static bool
check_device_suitability(VkPhysicalDevice &physical_device,
                         VkSurfaceKHR &surface)
{
    VkPhysicalDeviceProperties physical_device_props;
    VkPhysicalDeviceFeatures physical_device_feats;

    vkGetPhysicalDeviceProperties(physical_device, &physical_device_props);
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_feats);

    log::debug(std::format("Found GPU: {}", physical_device_props.deviceName));

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

    DeviceSwapchainSupportDetails swapchain_support_details =
        get_swapchain_support_details(surface, physical_device);

    bool swapchain_support = swapchain_support_details.surface_format_count > 0 &&
                             swapchain_support_details.present_mode_count > 0;

    return graphics_queue_support &&
           extension_support &&
           swapchain_support;
};

static DeviceQueueFamilyIndices
get_queue_family_indices(VkPhysicalDevice &physical_device)
{
    DeviceQueueFamilyIndices queue_family_indices = {};

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count,
                                             nullptr);

    VkQueueFamilyProperties queue_family_props[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count,
                                             queue_family_props);

    for (u32 i = 0; i < queue_family_count; ++i) {
        if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_indices.graphics = i;
        }
    }

    return queue_family_indices;
}

static bool
check_extension_support(VkPhysicalDevice &physical_device)
{
    u32 extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    VkExtensionProperties extension_props[extension_count];
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_props);

    // NOTE: meh
    bool extension_support[enabled_extension_count] = {false};

    for (u32 i = 0; i < enabled_extension_count; ++i) {
        for (auto &extension : extension_props) {
            if (strcmp(extension.extensionName, enabled_extensions[i])) {
                extension_support[i] = true;
            }
        }
    }

    for (u32 i = 0; i < enabled_extension_count; ++i) {
        if (!extension_support[i]) {
            log::error(std::format("Missing support for required extension: {}",
                                   enabled_extensions[i]));

            return false;
        }
    }

    return true;
}

// NOTE: i kinda hate this
static DeviceSwapchainSupportDetails
get_swapchain_support_details(VkSurfaceKHR &surface,
                              VkPhysicalDevice &physical_device)

{
    u32 surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &surface_format_count,
        nullptr);

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &present_mode_count,
        nullptr);

    DeviceSwapchainSupportDetails swapchain_support_details =
        DeviceSwapchainSupportDetails(surface_format_count, present_mode_count);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &swapchain_support_details.surface_capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &surface_format_count,
        swapchain_support_details.surface_formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &present_mode_count,
        swapchain_support_details.present_modes);

    return swapchain_support_details;
}

static VkSurfaceFormatKHR
select_surface_format(u32 available_surface_format_count,
                      VkSurfaceFormatKHR *available_surface_formats)
{
    for (u32 i = 0; i < available_surface_format_count; ++i) {
        VkSurfaceFormatKHR surface_format = available_surface_formats[i];
        if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surface_format;
        }
    }

    // NOTE: just settling for the first available format as a fallback
    return available_surface_formats[0];
}

static VkPresentModeKHR
select_present_mode(u32 available_present_mode_count,
                    VkPresentModeKHR *available_present_modes)
{
    for (u32 i = 0; i < available_present_mode_count; ++i) {
        VkPresentModeKHR present_mode = available_present_modes[i];
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    // NOTE: just settling for fifo as a fallback
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D
select_swap_extent(SDL_Window *window, VkSurfaceCapabilitiesKHR &surface_capabilities)
{
    i32 width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);

    VkExtent2D extent = {
        CLAMP((u32)width,
              surface_capabilities.maxImageExtent.width,
              surface_capabilities.minImageExtent.width),
        CLAMP((u32)height,
              surface_capabilities.maxImageExtent.height,
              surface_capabilities.minImageExtent.height),
    };

    return extent;
}

DeviceSwapchainSupportDetails::DeviceSwapchainSupportDetails(u32 surface_format_count,
                                                             u32 present_mode_count)
    : surface_format_count(surface_format_count),
      present_mode_count(present_mode_count)
{
    surface_formats = (VkSurfaceFormatKHR *)malloc(surface_format_count / sizeof(usize));
    present_modes = (VkPresentModeKHR *)malloc(present_mode_count / sizeof(usize));
}

DeviceSwapchainSupportDetails::~DeviceSwapchainSupportDetails()
{
    free(surface_formats);
    free(present_modes);
}

} // namespace gp

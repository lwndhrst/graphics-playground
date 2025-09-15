#include "goose/render/render.hpp"

#include "goose/core/util.hpp"
#include "goose/render/device.hpp"

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

namespace goose::render {

bool
create_instance(RenderData *data, const char *app_name, u32 app_version)
{
    VkResult result;

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name,
        .applicationVersion = app_version,
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    // TODO: Properly check layer/extension support

#ifdef GOOSE_DEBUG
    data->instance_layers.push_back(VALIDATION_LAYER_NAME);
    data->instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<u32>(data->instance_layers.size()),
        .ppEnabledLayerNames = data->instance_layers.data(),
        .enabledExtensionCount = static_cast<u32>(data->instance_extensions.size()),
        .ppEnabledExtensionNames = data->instance_extensions.data(),
    };

    result = vkCreateInstance(&instance_create_info, nullptr, &data->instance);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Vulkan error: {}", string_VkResult(result));
        return false;
    }

    return true;
}

bool
init(RenderData *data)
{
    if (data->surface == nullptr)
    {
        LOG_ERROR("Missing surface");
        return false;
    }

    // TODO: Which device extensions are required?
    data->device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkPhysicalDevice gpu = get_gpu(data->instance, data->surface, data->device_extensions);
    if (gpu == nullptr)
    {
        LOG_ERROR("No suitable GPU found");
        return false;
    }

#ifdef GOOSE_DEBUG
    // NOTE: Modern Vulkan doesn't seem to distinguish between instance and device layers anymore.
    //       This will just be ignored on modern Vulkan versions, but leaving this here anyway.
    data->device_layers.push_back(VALIDATION_LAYER_NAME);
#endif

    data->device = create_logical_device(gpu, data->surface, data->device_layers, data->device_extensions);
    if (data->device == nullptr)
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    return true;
}

void
cleanup(RenderData *data)
{
    vkDeviceWaitIdle(data->device);

    vkDestroyDevice(data->device, nullptr);
    vkDestroySurfaceKHR(data->instance, data->surface, nullptr);
    vkDestroyInstance(data->instance, nullptr);
}

} // namespace goose::render

#include "goose/render/instance.hpp"

#include "goose/core/util.hpp"

VkInstance
goose::render::create_instance(
    const char *app_name,
    u32 app_version,
    const std::vector<const char *> &layers,
    const std::vector<const char *> &extensions)
{
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name,
        .applicationVersion = app_version,
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    // TODO: Properly check layer/extension support

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<u32>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<u32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance;
    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);

        // TODO: Error handling
    }

    return instance;
}

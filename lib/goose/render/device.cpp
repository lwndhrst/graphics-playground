#include "goose/render/device.hpp"

#include "goose/core/util.hpp"

namespace goose::render {

QueueFamilyIndices
get_queue_families(VkPhysicalDevice gpu)
{
    u32 queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queue_family_count, nullptr);

    VkQueueFamilyProperties2 queue_family_properties = {};
    queue_family_properties.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;

    std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count, queue_family_properties);
    vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queue_family_count, queue_families.data());

    QueueFamilyIndices indices;

    for (u32 i = 0; i < queue_family_count; ++i)
    {
        if (queue_families[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics = i;
            return indices;
        }
    }

    return indices;
}

VkPhysicalDevice
get_gpu(VkInstance instance, const std::vector<const char *> &extensions)
{
    VkResult result;

    u32 device_count;
    result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> devices(device_count);
    result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (const auto &device : devices)
    {
        VkPhysicalDeviceProperties2 device_properties = {};
        device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(device, &device_properties);

        VkPhysicalDeviceFeatures2 device_features = {};
        device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        vkGetPhysicalDeviceFeatures2(device, &device_features);

        LOG_INFO("Found GPU: {}", device_properties.properties.deviceName);

        QueueFamilyIndices indices = get_queue_families(device);

        // TODO: Proper suitability check?
        if (device_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            indices.graphics.has_value())
        {
            LOG_INFO("Using GPU: {}", device_properties.properties.deviceName);
            return device;
        }
    }

    LOG_ERROR("No suitable GPU found");
    return nullptr;
}

VkDevice
create_logical_device(VkPhysicalDevice gpu,
                      const std::vector<const char *> &layers,
                      const std::vector<const char *> &extensions)
{
    QueueFamilyIndices indices = get_queue_families(gpu);

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphics.value();
    queue_create_info.queueCount = 1;

    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceVulkan12Features device_features_12 = {};
    device_features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    device_features_12.bufferDeviceAddress = true;
    device_features_12.descriptorIndexing = true;

    VkPhysicalDeviceVulkan13Features device_features_13 = {};
    device_features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    device_features_13.pNext = &device_features_12;
    device_features_13.dynamicRendering = true;
    device_features_13.synchronization2 = true;

    VkPhysicalDeviceFeatures2 device_features = {};
    device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features.pNext = &device_features_13;

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features.features;
    device_create_info.enabledLayerCount = static_cast<u32>(layers.size());
    device_create_info.ppEnabledLayerNames = layers.data();
    device_create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
    device_create_info.ppEnabledExtensionNames = extensions.data();

    VkDevice logical_device;
    VkResult result = vkCreateDevice(gpu, &device_create_info, nullptr, &logical_device);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create logical device");
    }

    return logical_device;
}

} // namespace goose::render

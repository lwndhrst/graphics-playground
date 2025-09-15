#include "goose/render/device.hpp"

#include "goose/core/util.hpp"

namespace goose::render {

bool
QueueFamilyIndices::is_complete()
{
    return graphics.has_value() && present.has_value();
}

QueueFamilyIndices
get_queue_families(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    u32 queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queue_family_count, nullptr);

    VkQueueFamilyProperties2 queue_family_properties = {
        .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2,
    };

    std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count, queue_family_properties);
    vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queue_family_count, queue_families.data());

    QueueFamilyIndices indices;

    for (u32 i = 0; i < queue_family_count; ++i)
    {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &present_support);
        if (present_support)
        {
            indices.present = i;
        }

        if (queue_families[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics = i;
        }

        if (indices.is_complete())
        {
            break;
        }
    }

    return indices;
}

VkPhysicalDevice
get_gpu(VkInstance instance,
        VkSurfaceKHR surface,
        const std::vector<const char *> &extensions)
{
    u32 gpu_count;
    vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);

    std::vector<VkPhysicalDevice> gpus(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

    for (const auto &gpu : gpus)
    {
        VkPhysicalDeviceProperties2 gpu_properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        };

        vkGetPhysicalDeviceProperties2(gpu, &gpu_properties);

        VkPhysicalDeviceFeatures2 gpu_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        };

        vkGetPhysicalDeviceFeatures2(gpu, &gpu_features);

        LOG_INFO("Found GPU: {}", gpu_properties.properties.deviceName);

        QueueFamilyIndices indices = get_queue_families(gpu, surface);

        // TODO: Proper suitability check?
        if (gpu_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            indices.is_complete())
        {
            LOG_INFO("Using GPU: {}", gpu_properties.properties.deviceName);
            return gpu;
        }
    }

    return nullptr;
}

VkDevice
create_logical_device(VkPhysicalDevice gpu,
                      VkSurfaceKHR surface,
                      const std::vector<const char *> &layers,
                      const std::vector<const char *> &extensions)
{
    QueueFamilyIndices indices = get_queue_families(gpu, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<u32> unique_queue_families = {
        indices.graphics.value(),
        indices.present.value(),
    };

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    for (const auto &queue_family : unique_queue_families)
    {
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceVulkan12Features device_features_12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .bufferDeviceAddress = true,
    };

    VkPhysicalDeviceVulkan13Features device_features_13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &device_features_12,
        .synchronization2 = true,
        .dynamicRendering = true,
    };

    VkPhysicalDeviceFeatures2 device_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &device_features_13,
    };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = static_cast<u32>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<u32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = &device_features.features,
    };

    VkDevice device;
    VkResult result = vkCreateDevice(gpu, &device_create_info, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        // TODO: Error handling
    }

    return device;
}

} // namespace goose::render

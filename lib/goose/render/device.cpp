#include "goose/render/device.hpp"

#include "goose/common/util.hpp"
#include "goose/render/instance.hpp"
#include "goose/render/swapchain.hpp"

struct QueueFamilyIndices {
    std::optional<u32> graphics;
    std::optional<u32> present;
    std::optional<u32> compute;

    // TODO: More queue families

    bool is_complete()
    {
        return graphics.has_value() && present.has_value() && compute.has_value();
    }
};

static QueueFamilyIndices
get_queue_family_indices(VkPhysicalDevice gpu, VkSurfaceKHR surface)
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

        if (queue_families[i].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.compute = i;
        }

        if (indices.is_complete())
        {
            break;
        }
    }

    return indices;
}

static u32
score_gpu(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features)
{
    u32 score = 0;

    LOG_INFO("Checking GPU: {}", properties.deviceName);

    // TODO: Figure out proper scoring

    switch (properties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 10;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 1;
        break;
    default:
        break;
    }

    return score;
}

static bool
check_extension_support(VkPhysicalDevice gpu, const std::vector<const char *> &extensions)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(extensions.begin(), extensions.end());

    for (const auto &extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

static VkPhysicalDevice
get_gpu(
    VkInstance instance,
    VkSurfaceKHR surface,
    const std::vector<const char *> &extensions)
{
    u32 gpu_count;
    vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);

    std::vector<VkPhysicalDevice> gpus(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

    VkPhysicalDevice chosen_gpu = nullptr;
    u32 chosen_gpu_score = 0;
    VkPhysicalDeviceProperties chosen_gpu_properties;
    VkPhysicalDeviceFeatures chosen_gpu_features;

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

        u32 gpu_score = score_gpu(gpu_properties.properties, gpu_features.features);
        QueueFamilyIndices indices = get_queue_family_indices(gpu, surface);
        goose::render::SwapchainSupportDetails swapchain_support = goose::render::get_swapchain_support_details(gpu, surface);

        if (gpu_score > chosen_gpu_score &&
            indices.is_complete() &&
            check_extension_support(gpu, extensions) &&
            !swapchain_support.formats.empty() &&
            !swapchain_support.present_modes.empty())
        {
            chosen_gpu = gpu;
            chosen_gpu_score = gpu_score;
            chosen_gpu_properties = gpu_properties.properties;
            chosen_gpu_features = gpu_features.features;
        }
    }

    if (chosen_gpu != nullptr)
    {
        LOG_INFO("Using GPU: {}", chosen_gpu_properties.deviceName);
    }

    return chosen_gpu;
}

bool
goose::render::create_device(VkSurfaceKHR surface, Device &device)
{
    const Instance &instance = get_instance();
    ASSERT(instance.handle != VK_NULL_HANDLE, "Instance is not initialized");

#ifdef GOOSE_DEBUG
    // NOTE: Modern Vulkan doesn't seem to distinguish between instance and device layers anymore.
    //       This will just be ignored on modern Vulkan versions, but leaving this here anyway.
    device.layers.insert(device.layers.end(), instance.layers.begin(), instance.layers.end());
#endif

    // TODO: Which device extensions?
    device.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    device.physical = get_gpu(instance.handle, surface, device.extensions);
    if (device.physical == VK_NULL_HANDLE)
    {
        LOG_ERROR("No suitable GPU found");
        return false;
    }

    QueueFamilyIndices indices = get_queue_family_indices(device.physical, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<u32> unique_queue_family_indices = {
        indices.graphics.value(),
        indices.present.value(),
        indices.compute.value(),
    };

    // TODO: Support multiple queues
    u32 queue_count = 1;
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = queue_count,
        .pQueuePriorities = &queue_priority,
    };

    for (const auto &queue_family_index : unique_queue_family_indices)
    {
        queue_create_info.queueFamilyIndex = queue_family_index;
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

    // NOTE: Need to enable at least one feature for the pNext chain to work
    VkPhysicalDeviceFeatures2 device_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &device_features_13,
    };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &device_features_13,
        .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = static_cast<u32>(device.layers.size()),
        .ppEnabledLayerNames = device.layers.data(),
        .enabledExtensionCount = static_cast<u32>(device.extensions.size()),
        .ppEnabledExtensionNames = device.extensions.data(),
    };

    VkResult result = vkCreateDevice(device.physical, &device_create_info, nullptr, &device.logical);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);
        return false;
    }

    // TODO: How many queues and from which families?
    //       Currently only getting one queue per family; families are likely all the same anyway

    device.queue_families.graphics.index = indices.graphics.value();
    device.queue_families.graphics.queues.resize(queue_count);

    device.queue_families.present.index = indices.present.value();
    device.queue_families.present.queues.resize(queue_count);

    device.queue_families.compute.index = indices.compute.value();
    device.queue_families.compute.queues.resize(queue_count);

    for (const auto &queue_family_index : unique_queue_family_indices)
    {
        if (queue_family_index == device.queue_families.graphics.index)
        {
            vkGetDeviceQueue(device.logical, queue_family_index, 0, &device.queue_families.graphics.queues[0]);
        }

        if (queue_family_index == device.queue_families.present.index)
        {
            vkGetDeviceQueue(device.logical, queue_family_index, 0, &device.queue_families.present.queues[0]);
        }

        if (queue_family_index == device.queue_families.compute.index)
        {
            vkGetDeviceQueue(device.logical, queue_family_index, 0, &device.queue_families.compute.queues[0]);
        }
    }

    return true;
}

void
goose::render::destroy_device(Device &device)
{
    vkDestroyDevice(device.logical, nullptr);
}

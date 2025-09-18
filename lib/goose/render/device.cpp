#include "goose/render/device.hpp"

#include "goose/common/util.hpp"
#include "goose/render/instance.hpp"
#include "goose/render/swapchain.hpp"

struct QueueFamilyIndices {
    std::optional<u32> graphics;
    std::optional<u32> present;

    // TODO: More queue families

    bool is_complete()
    {
        return graphics.has_value() && present.has_value();
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

goose::render::Device
goose::render::create_device(
    const Instance &instance,
    VkSurfaceKHR surface,
    const std::vector<const char *> &layers,
    const std::vector<const char *> &extensions)
{
    VkPhysicalDevice gpu = get_gpu(instance.handle, surface, extensions);
    if (gpu == VK_NULL_HANDLE)
    {
        LOG_ERROR("No suitable GPU found");
        return {};
    }

    QueueFamilyIndices indices = get_queue_family_indices(gpu, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<u32> unique_queue_families = {
        indices.graphics.value(),
        indices.present.value(),
    };

    // TODO: Support multiple queues
    u32 queue_count = 1;
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = queue_count,
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
        VK_LOG_ERROR(result);

        // TODO: Error handling
    }

    // TODO: How many queues and from which families?
    u32 graphics_queue_count = queue_count;
    Device::QueueFamily graphics_family = {
        .index = indices.graphics.value(),
        .queues = std::vector<VkQueue>(graphics_queue_count),
    };

    for (usize i = 0; i < graphics_queue_count; ++i)
    {
        vkGetDeviceQueue(device, graphics_family.index, i, &graphics_family.queues[i]);
    }

    u32 present_queue_count = queue_count;
    Device::QueueFamily present_family = {
        .index = indices.present.value(),
        .queues = std::vector<VkQueue>(present_queue_count),
    };

    for (usize i = 0; i < present_queue_count; ++i)
    {
        vkGetDeviceQueue(device, present_family.index, i, &present_family.queues[i]);
    }

    return {
        .physical = gpu,
        .logical = device,
        .layers = layers,
        .extensions = extensions,
        .queue_families = {
            .graphics = graphics_family,
            .present = present_family,
        },
    };
}

void
goose::render::destroy_device(Device &device)
{
    vkDestroyDevice(device.logical, nullptr);
}

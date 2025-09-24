#pragma once

#include "goose/common/assert.hpp"
#include "goose/common/types.hpp"

namespace goose::render {

struct QueueFamily {
    u32 index;
    std::vector<VkQueue> queues;
};

struct QueueFamilies {
    QueueFamily graphics;
    QueueFamily present;
    QueueFamily compute;
};

struct Device {
    inline static bool s_initialized;

    inline static VkPhysicalDevice s_physical_device;
    inline static VkDevice s_device;
    inline static QueueFamilies s_queue_families;

    static const VkPhysicalDevice &get_physical()
    {
        ASSERT(s_initialized, "Vulkan device is not initialized");
        return s_physical_device;
    };

    static const VkDevice &get()
    {
        ASSERT(s_initialized, "Vulkan device is not initialized");
        return s_device;
    };

    static const QueueFamilies &get_queue_families()
    {
        ASSERT(s_initialized, "Vulkan device is not initialized");
        return s_queue_families;
    };
};

bool create_device(VkSurfaceKHR surface);
void destroy_device();

} // namespace goose::render

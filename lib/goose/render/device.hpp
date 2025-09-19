#pragma once

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
    VkPhysicalDevice physical;
    VkDevice logical;
    std::vector<const char *> layers;
    std::vector<const char *> extensions;

    QueueFamilies queue_families;
};

bool create_device(const Instance &instance, VkSurfaceKHR surface, Device &device);
void destroy_device(Device &device);

} // namespace goose::render

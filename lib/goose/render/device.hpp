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

Device create_device(const Instance &instance, VkSurfaceKHR surface, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);
void destroy_device(Device &device);

} // namespace goose::render

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

bool create_device(VkSurfaceKHR surface);
void destroy_device();

const Device &get_device();

} // namespace goose::render

#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct Device {
    VkPhysicalDevice physical;
    VkDevice logical;
    std::vector<const char *> layers;
    std::vector<const char *> extensions;

    struct QueueFamily {
        u32 index;
        std::vector<VkQueue> queues;
    };

    struct {
        QueueFamily graphics;
        QueueFamily present;
    } queue_families;
};

Device create_device(const Instance &instance, VkSurfaceKHR surface, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);
void destroy_device(Device &device);

} // namespace goose::render

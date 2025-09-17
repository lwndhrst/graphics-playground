#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

struct Device {
    VkPhysicalDevice physical;
    VkDevice logical;
    std::vector<const char *> layers;
    std::vector<const char *> extensions;

    struct Queues {
        VkQueue graphics;
        VkQueue present;
    } queues;
};

struct QueueFamilyIndices {
    std::optional<u32> graphics;
    std::optional<u32> present;

    // TODO: More queue families

    bool is_complete()
    {
        return graphics.has_value() && present.has_value();
    }
};

QueueFamilyIndices get_queue_family_indices(VkPhysicalDevice gpu, VkSurfaceKHR surface);

Device create_device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);

} // namespace goose::render

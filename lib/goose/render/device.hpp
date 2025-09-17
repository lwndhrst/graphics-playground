#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

struct DeviceQueues {
    VkQueue graphics;
    VkQueue present;
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

VkPhysicalDevice get_gpu(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char *> &extensions);

std::pair<VkDevice, DeviceQueues> create_logical_device(VkPhysicalDevice gpu, VkSurfaceKHR surface, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);

} // namespace goose::render

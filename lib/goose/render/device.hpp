#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

struct QueueFamilyIndices {
    std::optional<u32> graphics;
    std::optional<u32> present;

    // TODO: More queue families

    bool is_complete();
};

QueueFamilyIndices get_queue_families(VkPhysicalDevice gpu, VkSurfaceKHR surface);

VkPhysicalDevice get_gpu(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char *> &extensions);

VkDevice create_logical_device(VkPhysicalDevice gpu, VkSurfaceKHR surface, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);

} // namespace goose::render

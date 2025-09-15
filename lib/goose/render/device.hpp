#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

struct QueueFamilyIndices {
    std::optional<u32> graphics;

    // TODO: More queue families
};

QueueFamilyIndices get_queue_families(VkPhysicalDevice gpu);

VkPhysicalDevice get_gpu(VkInstance instance, const std::vector<const char *> &extensions);

VkDevice create_logical_device(VkPhysicalDevice gpu, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);

} // namespace goose::render

#pragma once

#include "goose/core/types.hpp"
#include "goose/render/render.hpp"

namespace goose::render {

VkPhysicalDevice get_gpu(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char *> &extensions);

VkDevice create_logical_device(VkPhysicalDevice gpu, VkSurfaceKHR surface, const std::vector<const char *> &layers, const std::vector<const char *> &extensions, RenderContext::Queues &queues);

} // namespace goose::render

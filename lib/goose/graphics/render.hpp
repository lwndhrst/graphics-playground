#pragma once

#include "goose/core/types.hpp"

namespace goose::graphics {

struct RenderData {
    VkExtent2D window_extent;

    VkInstance instance;
    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;

    VkSurfaceKHR surface;
};

bool create_instance(RenderData *data, const char *app_name, u32 app_version);

bool init(RenderData *data, VkSurfaceKHR surface);

void cleanup(RenderData *data);

} // namespace goose::graphics

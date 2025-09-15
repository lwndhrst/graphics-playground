#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

struct RenderData {
    VkExtent2D window_extent;

    VkInstance instance;
    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;

    VkDevice device;
    std::vector<const char *> device_layers;
    std::vector<const char *> device_extensions;

    VkSurfaceKHR surface;
};

bool create_instance(RenderData *data, const char *app_name, u32 app_version);

bool init(RenderData *data);

void cleanup(RenderData *data);

} // namespace goose::render

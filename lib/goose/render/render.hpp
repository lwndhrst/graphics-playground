#pragma once

#include "goose/core/types.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"
#include "goose/render/swapchain.hpp"

namespace goose::render {

struct RenderContext {
    struct Window {
        VkExtent2D extent;
        VkSurfaceKHR surface;
    };

    Window window;
    Instance instance;
    Device device;
    Swapchain swapchain;
};

bool create_instance(RenderContext *ctx, const char *app_name, u32 app_version);

bool init(RenderContext *ctx, VkExtent2D window_extent, VkSurfaceKHR surface);

void cleanup(RenderContext *ctx);

} // namespace goose::render

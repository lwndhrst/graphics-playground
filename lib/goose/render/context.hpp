#pragma once

#include "goose/common/types.hpp"
#include "goose/render/device.hpp"
#include "goose/render/swapchain.hpp"

namespace goose::render {

struct RenderContext {
    Device device;
    Swapchain swapchain;
};

bool create_render_context(const Window &window, RenderContext &ctx);
void destroy_render_context(RenderContext &ctx);

} // namespace goose::render

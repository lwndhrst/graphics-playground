#pragma once

#include "goose/common/types.hpp"
#include "goose/render/device.hpp"
#include "goose/render/frame.hpp"
#include "goose/render/swapchain.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

namespace goose::render {

struct RenderContext {
    Device device;

    Swapchain swapchain;

    Frame frames[MAX_FRAMES_IN_FLIGHT];
    u32 current_frame;
};

bool create_render_context(const Window &window, RenderContext &ctx);
void destroy_render_context(RenderContext &ctx);

void begin_frame(RenderContext &ctx);
void end_frame(RenderContext &ctx);

} // namespace goose::render

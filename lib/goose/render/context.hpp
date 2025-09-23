#pragma once

#include "goose/common/types.hpp"
#include "goose/render/frame.hpp"
#include "goose/render/swapchain.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

namespace goose::render {

struct RenderContext {
    Swapchain swapchain;
    u32 current_swapchain_image;

    Frame frames[MAX_FRAMES_IN_FLIGHT];
    u32 current_frame;

    std::vector<std::function<void()>> cleanup_callbacks;
};

bool create_render_context(RenderContext &ctx, const Window &window);
void destroy_render_context(RenderContext &ctx);

void add_cleanup_callback(RenderContext &ctx, const std::function<void()> &&callback);

void resize_swapchain(RenderContext &ctx, const Window &window);

std::pair<VkCommandBuffer, const SwapchainImage &> begin_frame(RenderContext &ctx);
void end_frame(RenderContext &ctx);

} // namespace goose::render

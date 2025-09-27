#pragma once

#include "goose/common/types.hpp"
#include "goose/render/frame.hpp"
#include "goose/render/immediate.hpp"
#include "goose/render/swapchain.hpp"

namespace goose::render {

struct RenderContext {
    SwapchainInfo swapchain;
    u32 current_swapchain_image;

    FrameData frame_data;

    ImmediateData immediate_data;

    std::vector<std::function<void()>> cleanup_callbacks;
};

bool create_render_context(RenderContext &ctx, const WindowInfo &window, const FrameCreateInfo &frame_create_info);
void destroy_render_context(RenderContext &ctx);

void add_cleanup_callback(RenderContext &ctx, const std::function<void()> &&callback);

void resize_swapchain(RenderContext &ctx, const WindowInfo &window);

std::pair<const Frame &, const SwapchainImageInfo &> begin_frame(RenderContext &ctx);
void end_frame(RenderContext &ctx);

VkCommandBuffer begin_immediate(const RenderContext &ctx);
void end_immediate(const RenderContext &ctx);

} // namespace goose::render

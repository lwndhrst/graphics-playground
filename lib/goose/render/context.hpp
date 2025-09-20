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
    u32 current_swapchain_image;

    Frame frames[MAX_FRAMES_IN_FLIGHT];
    u32 current_frame;

    // 1 per frame in flight
    std::vector<VkFence> in_flight_fences;
    std::vector<VkSemaphore> image_available_semaphores;

    // 1 per swapchain image
    std::vector<VkSemaphore> render_finished_semaphores;
};

bool create_render_context(const Window &window, RenderContext &ctx);
void destroy_render_context(RenderContext &ctx);

void begin_frame(RenderContext &ctx);
void end_frame(RenderContext &ctx);

} // namespace goose::render

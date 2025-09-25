#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct FrameData {
    // TODO: How to deal with multiple pools and buffers for multi-threading?
    //       Additional secondary command buffers?
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    // Signaled when the previous queue submit call of this frame is finished
    VkFence in_flight_fence;

    // Signaled when the aquired swapchain image is actually available for rendering
    VkSemaphore image_available_semaphore;
};

bool create_frame(FrameData &frame);
void destroy_frame(FrameData &frame);

} // namespace goose::render

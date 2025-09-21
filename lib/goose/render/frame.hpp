#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct Frame {
    // TODO: How to deal with multiple pools and buffers for multi-threading?
    //       Additional secondary command buffers?
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    // Used to wait for the previous queue submit call of this frame to finish
    VkFence in_flight_fence;

    // Used to wait for the aquired swapchain image to be available for rendering
    VkSemaphore image_available_semaphore;
};

bool create_frame(const Device &device, Frame &frame);
void destroy_frame(const Device &device, Frame &frame);

void begin_command_buffer(Frame &frame);
void end_command_buffer(Frame &frame);

} // namespace goose::render

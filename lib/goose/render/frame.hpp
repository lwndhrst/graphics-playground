#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct Frame {
    // TODO: How to deal with multiple pools and buffers for multi-threading?
    //       Additional secondary command buffers?
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkFence in_flight_fence;
    VkSemaphore present_complete_semaphore;
    VkSemaphore render_finished_semaphore;
};

bool create_frame(const Device &device, Frame &frame);
void destroy_frame(const Device &device, Frame &frame);

} // namespace goose::render

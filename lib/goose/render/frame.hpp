#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct Frame {
    // TODO: How to deal with multiple pools and buffers for multi-threading?
    //       Additional secondary command buffers?
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
};

bool create_frame(const Device &device, Frame &frame);
void destroy_frame(const Device &device, Frame &frame);

void begin_command_buffer(Frame &frame);
void end_command_buffer(Frame &frame);

} // namespace goose::render

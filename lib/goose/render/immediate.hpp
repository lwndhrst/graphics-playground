#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct ImmediateData {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkFence in_flight_fence;
};

bool create_immediate(ImmediateData &immediate);
void destroy_immediate(ImmediateData &immediate);

} // namespace goose::render

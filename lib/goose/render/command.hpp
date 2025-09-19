#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct CommandPool {
    VkCommandPool handle;
};

bool create_command_pool(const Device &device, CommandPool &command_pool);
void destroy_command_pool(CommandPool &command_pool);

} // namespace goose::render

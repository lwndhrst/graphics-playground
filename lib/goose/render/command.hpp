#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct CommandBuffer {};

struct CommandPool {};

CommandPool create_command_pool();
void destroy_command_pool();

} // namespace goose::render

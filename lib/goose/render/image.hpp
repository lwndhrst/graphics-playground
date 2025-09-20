#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

void transition_image(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

}

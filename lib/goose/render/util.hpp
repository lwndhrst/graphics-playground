#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

VkFence create_fence(VkDevice device, VkFenceCreateFlags flags = 0);
VkSemaphore create_semaphore(VkDevice device, VkSemaphoreCreateFlags flags = 0);

void transition_image(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

} // namespace goose::render

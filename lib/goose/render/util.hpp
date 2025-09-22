#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

VkCommandPool create_command_pool(VkDevice device, u32 queue_family_index, VkCommandPoolCreateFlags flags = 0);

VkCommandBuffer alloc_command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel buffer_level);
VkCommandBufferSubmitInfo make_command_buffer_submit_info(VkCommandBuffer command_buffer);

VkFence create_fence(VkDevice device, VkFenceCreateFlags flags = 0);

VkSemaphore create_semaphore(VkDevice device, VkSemaphoreCreateFlags flags = 0);
VkSemaphoreSubmitInfo make_semaphore_submit_info(VkSemaphore semaphore, VkPipelineStageFlags2 stage_flags);

VkImageCreateInfo make_image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
VkImageViewCreateInfo make_image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
VkImageSubresourceRange make_image_subresource_range(VkImageAspectFlags aspect_flags);

void transition_image(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
void copy_image_to_image(VkCommandBuffer command_buffer, VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size);

} // namespace goose::render

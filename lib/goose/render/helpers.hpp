#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

VkCommandPool create_command_pool(u32 queue_family_index, VkCommandPoolCreateFlags command_pool_create_flags = 0);
void destroy_command_pool(VkCommandPool command_pool);

VkCommandBuffer allocate_command_buffer(VkCommandPool command_pool, VkCommandBufferLevel buffer_level);
VkCommandBufferBeginInfo make_command_buffer_begin_info(VkCommandBufferUsageFlags usage_flags);
VkCommandBufferSubmitInfo make_command_buffer_submit_info(VkCommandBuffer command_buffer);

VkDescriptorPool create_descriptor_pool(u32 max_descriptor_sets, std::span<VkDescriptorPoolSize> descriptor_pool_sizes);
void destroy_descriptor_pool(VkDescriptorPool descriptor_pool);
void destroy_descriptor_set_layout(VkDescriptorSetLayout descriptor_set_layout);
VkDescriptorSet allocate_descriptor_set(VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout);

VkFence create_fence(VkFenceCreateFlags fence_create_flags = 0);

VkSemaphore create_semaphore(VkSemaphoreCreateFlags semaphore_create_flags = 0);
VkSemaphoreSubmitInfo make_semaphore_submit_info(VkSemaphore semaphore, VkPipelineStageFlags2 stage_flags);

VkImageCreateInfo make_image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
VkImageViewCreateInfo make_image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
VkImageSubresourceRange make_image_subresource_range(VkImageAspectFlags aspect_flags);

VkShaderModule create_shader_module(const std::string &file_path);
void destroy_shader_module(VkShaderModule shader_module);

void transition_image(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
void copy_image_to_image(VkCommandBuffer command_buffer, VkImage src_image, VkImage dst_image, VkExtent2D src_extent, VkExtent2D dst_extent);

} // namespace goose::render

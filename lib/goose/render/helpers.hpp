#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

VkCommandPool create_command_pool(u32 queue_family_index, VkCommandPoolCreateFlags create_flags = 0);
void destroy_command_pool(VkCommandPool pool);
VkCommandBuffer allocate_command_buffer(VkCommandPool pool, VkCommandBufferLevel level);

VkDescriptorPool create_descriptor_pool(u32 max_sets, std::span<VkDescriptorPoolSize> pool_sizes);
void destroy_descriptor_pool(VkDescriptorPool pool);
VkDescriptorSet allocate_descriptor_set(VkDescriptorPool pool, VkDescriptorSetLayout layout);

VkFence create_fence(VkFenceCreateFlags create_flags = 0);
void destroy_fence(VkFence fence);

VkSemaphore create_semaphore(VkSemaphoreCreateFlags create_flags = 0);
void destroy_semaphore(VkSemaphore semaphore);

VkShaderModule create_shader_module(const std::string &file_path);
void destroy_shader_module(VkShaderModule shader_module);

void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
void copy_image_to_image(VkCommandBuffer cmd, VkImage src_image, VkImage dst_image, VkExtent3D src_extent, VkExtent3D dst_extent);
void copy_image_to_image(VkCommandBuffer cmd, VkImage src_image, VkImage dst_image, VkExtent2D src_extent, VkExtent2D dst_extent);

} // namespace goose::render

#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct FrameCommandInfo {
    u32 queue_family_index;
    VkCommandPoolCreateFlags pool_create_flags;
    VkCommandBufferUsageFlags main_buffer_usage_flags;
};

struct FrameDescriptorInfo {
    // TODO
};

struct FrameImageInfo {
    // TODO
};

struct FrameSynchronizationInfo {
    u32 extra_fence_count;
    VkFenceCreateFlags *fence_create_flags;
    u32 extra_semaphore_count;
    VkSemaphoreCreateFlags *semaphore_create_flags;
};

struct FrameCreateInfo {
    u32 max_frames_in_flight;

    FrameCommandInfo *command_info;
    FrameDescriptorInfo *descriptor_info;
    FrameImageInfo *image_info;
    FrameSynchronizationInfo *sync_info;
};

struct FrameData {
    u32 max_frames_in_flight;
    u32 current_frame_index;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> main_command_buffers;

    // Signaled when the previous queue submit call of the current frame is finished
    std::vector<VkFence> in_flight_fences;

    // Signaled when the aquired swapchain image is actually available for rendering
    std::vector<VkSemaphore> image_available_semaphores;
};

bool create_frame_data(FrameData &frame_data, const FrameCreateInfo &create_info);
void destroy_frame_data(FrameData &frame_data);

} // namespace goose::render

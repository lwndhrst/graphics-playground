#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct FrameCommandInfo {
    u32 queue_family_index;
    VkCommandPoolCreateFlags pool_create_flags;
    u32 secondary_buffer_count;
};

struct FrameDescriptorInfo {
    // TODO
};

struct FrameImageInfo {
    // TODO
};

struct FrameCreateInfo {
    u32 max_frames_in_flight;

    FrameCommandInfo *command_info;
    FrameDescriptorInfo *descriptor_info;
    FrameImageInfo *image_info;
};

struct FrameData {
    u32 max_frames_in_flight;
    u32 current_frame_index;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> main_command_buffers;

    VkDescriptorPool descriptor_pool;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
    std::vector<VkDescriptorSet> descriptor_sets;

    // Signaled when the previous queue submit call of the current frame is finished
    std::vector<VkFence> in_flight_fences;

    // Signaled when the aquired swapchain image is actually available for rendering
    std::vector<VkSemaphore> image_available_semaphores;
};

bool create_frame_data(FrameData &frame_data, const FrameCreateInfo &create_info);
void destroy_frame_data(FrameData &frame_data);

struct Frame {
    // Always available
    VkCommandBuffer main_command_buffer;
    VkFence in_flight_fence;
    VkSemaphore image_available_semaphore;

    // Optional
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSet descriptor_set;
};

inline Frame
get_current_frame(const FrameData &frame_data)
{
    const u32 &i = frame_data.current_frame_index;

    return {
        .main_command_buffer = frame_data.main_command_buffers[i],
        .in_flight_fence = frame_data.in_flight_fences[i],
        .image_available_semaphore = frame_data.image_available_semaphores[i],
    };
}

} // namespace goose::render

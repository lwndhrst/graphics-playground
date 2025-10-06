#pragma once

#include "goose/common/types.hpp"
#include "goose/render/cleanup_queue.hpp"

namespace goose::render {

struct FrameDataCreateInfo {
    u32 max_frames_in_flight;
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

    // Cleared at the start of each frame
    std::vector<CleanupQueue> cleanup_queues;
};

bool create_frame_data(FrameData &frame_data, const FrameDataCreateInfo &create_info);
void destroy_frame_data(FrameData &frame_data);

inline void
increment_frame_index(FrameData &frame_data)
{
    frame_data.current_frame_index = (frame_data.current_frame_index + 1) % frame_data.max_frames_in_flight;
}

struct Frame {
    u32 index;
    VkCommandBuffer main_command_buffer;
    VkFence in_flight_fence;
    VkSemaphore image_available_semaphore;
    CleanupQueue &cleanup_queue;
};

inline Frame
get_current_frame(FrameData &frame_data)
{
    const u32 &i = frame_data.current_frame_index;

    return {
        .index = i,
        .main_command_buffer = frame_data.main_command_buffers[i],
        .in_flight_fence = frame_data.in_flight_fences[i],
        .image_available_semaphore = frame_data.image_available_semaphores[i],
        .cleanup_queue = frame_data.cleanup_queues[i],
    };
}

} // namespace goose::render

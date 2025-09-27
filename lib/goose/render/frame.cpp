#include "goose/render/frame.hpp"

#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"

bool
goose::render::create_frame_data(FrameData &frame_data, const FrameDataCreateInfo &create_info)
{
    frame_data.max_frames_in_flight = create_info.max_frames_in_flight;
    frame_data.current_frame_index = 0;

    frame_data.command_pool = create_command_pool(
        Device::get_queue_families().graphics.index,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    frame_data.main_command_buffers.resize(create_info.max_frames_in_flight);
    frame_data.in_flight_fences.resize(create_info.max_frames_in_flight);
    frame_data.image_available_semaphores.resize(create_info.max_frames_in_flight);

    for (u32 i = 0; i < create_info.max_frames_in_flight; ++i)
    {
        frame_data.main_command_buffers[i] = allocate_command_buffer(frame_data.command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        frame_data.in_flight_fences[i] = create_fence(VK_FENCE_CREATE_SIGNALED_BIT);
        frame_data.image_available_semaphores[i] = create_semaphore();
    }

    return true;
}

void
goose::render::destroy_frame_data(FrameData &frame_data)
{
    const VkDevice &device = Device::get();

    for (u32 i = 0; i < frame_data.max_frames_in_flight; ++i)
    {
        destroy_semaphore(frame_data.image_available_semaphores[i]);
        destroy_fence(frame_data.in_flight_fences[i]);
    }

    destroy_command_pool(frame_data.command_pool);

    frame_data = {};
}

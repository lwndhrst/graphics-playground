#include "goose/render/frame.hpp"

#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"

bool
goose::render::create_frame(Frame &frame)
{
    frame.command_pool = create_command_pool(
        Device::get_queue_families().graphics.index,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    frame.command_buffer = allocate_command_buffer(
        frame.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    frame.in_flight_fence = create_fence(VK_FENCE_CREATE_SIGNALED_BIT);
    frame.image_available_semaphore = create_semaphore();

    return true;
}

void
goose::render::destroy_frame(Frame &frame)
{
    const VkDevice &device = Device::get();

    vkDestroySemaphore(device, frame.image_available_semaphore, nullptr);
    vkDestroyFence(device, frame.in_flight_fence, nullptr);

    vkDestroyCommandPool(device, frame.command_pool, nullptr);

    frame = {};
}

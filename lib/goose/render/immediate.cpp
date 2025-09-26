#include "goose/render/immediate.hpp"

#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"

bool
goose::render::create_immediate_data(ImmediateData &immediate)
{
    immediate.command_pool = create_command_pool(
        Device::get_queue_families().graphics.index,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    immediate.command_buffer = allocate_command_buffer(
        immediate.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    immediate.in_flight_fence = create_fence();

    return true;
}

void
goose::render::destroy_immediate_data(ImmediateData &immediate)
{
    const VkDevice &device = Device::get();

    vkDestroyFence(device, immediate.in_flight_fence, nullptr);
    vkDestroyCommandPool(device, immediate.command_pool, nullptr);

    immediate = {};
}

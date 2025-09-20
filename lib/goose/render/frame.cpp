#include "goose/render/frame.hpp"

#include "goose/common/util.hpp"
#include "goose/render/device.hpp"

static VkCommandPool
create_command_pool(VkDevice device, u32 queue_family_index, VkCommandPoolCreateFlags flags = 0)
{
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool command_pool;
    VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);

    // TODO: Error handling
    VK_ASSERT(result);

    return command_pool;
}

// TODO: Add function for allocating multiple command buffers at once
static VkCommandBuffer
alloc_command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel command_buffer_level)
{
    ASSERT(command_pool != VK_NULL_HANDLE, "Command pool does not exist");

    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = command_buffer_level,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(device, &command_buffer_alloc_info, &command_buffer);

    // TODO: Error handling
    VK_ASSERT(result);

    return command_buffer;
}

static VkFence
create_fence(VkDevice device, VkFenceCreateFlags flags = 0)
{
    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = flags,
    };

    VkFence fence;
    VkResult result = vkCreateFence(device, &fence_create_info, nullptr, &fence);

    // TODO: Error handling
    VK_ASSERT(result);

    return fence;
}

static VkSemaphore
create_semaphore(VkDevice device, VkSemaphoreCreateFlags flags = 0)
{
    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .flags = flags,
    };

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphore);

    // TODO: Error handling
    VK_ASSERT(result);

    return semaphore;
}

bool
goose::render::create_frame(const Device &device, Frame &frame)
{
    frame.in_flight_fence = create_fence(device.logical, VK_FENCE_CREATE_SIGNALED_BIT);
    frame.image_available_semaphore = create_semaphore(device.logical);
    frame.render_finished_semaphore = create_semaphore(device.logical);

    frame.command_pool = create_command_pool(
        device.logical,
        device.queue_families.graphics.index,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    frame.command_buffer = alloc_command_buffer(
        device.logical,
        frame.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    return true;
}

void
goose::render::destroy_frame(const Device &device, Frame &frame)
{
    vkDestroyCommandPool(device.logical, frame.command_pool, nullptr);

    vkDestroySemaphore(device.logical, frame.render_finished_semaphore, nullptr);
    vkDestroySemaphore(device.logical, frame.image_available_semaphore, nullptr);
    vkDestroyFence(device.logical, frame.in_flight_fence, nullptr);
}

void
goose::render::begin_command_buffer(Frame &frame)
{
    vkResetCommandBuffer(frame.command_buffer, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(frame.command_buffer, &command_buffer_begin_info);
}

void
goose::render::end_command_buffer(Frame &frame)
{
    vkEndCommandBuffer(frame.command_buffer);
}

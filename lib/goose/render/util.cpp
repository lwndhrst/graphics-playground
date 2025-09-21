#include "goose/render/util.hpp"

#include "goose/common/log.hpp"

VkCommandPool
goose::render::create_command_pool(VkDevice device, u32 queue_family_index, VkCommandPoolCreateFlags flags)
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

VkCommandBuffer
goose::render::alloc_command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel buffer_level)
{
    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = buffer_level,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(device, &command_buffer_alloc_info, &command_buffer);

    // TODO: Error handling
    VK_ASSERT(result);

    return command_buffer;
}

VkCommandBufferSubmitInfo
goose::render::make_command_buffer_submit_info(VkCommandBuffer command_buffer)
{
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = command_buffer,
    };
}

VkFence
goose::render::create_fence(VkDevice device, VkFenceCreateFlags flags)
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

VkSemaphore
goose::render::create_semaphore(VkDevice device, VkSemaphoreCreateFlags flags)
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

VkSemaphoreSubmitInfo
goose::render::make_semaphore_submit_info(VkSemaphore semaphore, VkPipelineStageFlags2 stage_flags)
{
    return {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stage_flags,
    };
}

VkImageSubresourceRange
goose::render::make_image_subresource_range(VkImageAspectFlags aspect_flags)
{
    return {
        .aspectMask = aspect_flags,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
}

void
goose::render::transition_image(
    VkCommandBuffer command_buffer,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_flags =
        new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
            ? VK_IMAGE_ASPECT_DEPTH_BIT
            : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange subresource_range =
        make_image_subresource_range(aspect_flags);

    VkImageMemoryBarrier2 image_memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,

        .oldLayout = old_layout,
        .newLayout = new_layout,

        .image = image,
        .subresourceRange = subresource_range,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,

        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_memory_barrier,
    };

    vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

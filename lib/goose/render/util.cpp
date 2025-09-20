#include "goose/render/util.hpp"

#include "goose/common/log.hpp"

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

void
goose::render::transition_image(
    VkCommandBuffer command_buffer,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_mask =
        new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
            ? VK_IMAGE_ASPECT_DEPTH_BIT
            : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange subresource_range = {
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };

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

#include "goose/render/util.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"

VkCommandPool
goose::render::create_command_pool(u32 queue_family_index, VkCommandPoolCreateFlags command_pool_create_flags)
{
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = command_pool_create_flags,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool command_pool;
    VkResult result = vkCreateCommandPool(Device::get(), &command_pool_create_info, nullptr, &command_pool);

    // TODO: Error handling
    VK_ASSERT(result);

    return command_pool;
}

void
goose::render::destroy_command_pool(VkCommandPool command_pool)
{
    vkDestroyCommandPool(Device::get(), command_pool, nullptr);
}

VkCommandBuffer
goose::render::allocate_command_buffer(VkCommandPool command_pool, VkCommandBufferLevel command_buffer_level)
{
    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = command_buffer_level,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(Device::get(), &command_buffer_alloc_info, &command_buffer);

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

VkDescriptorPool
goose::render::create_descriptor_pool(u32 max_descriptor_sets, std::span<VkDescriptorPoolSize> descriptor_pool_sizes)
{
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = max_descriptor_sets,
        .poolSizeCount = static_cast<u32>(descriptor_pool_sizes.size()),
        .pPoolSizes = descriptor_pool_sizes.data(),
    };

    VkDescriptorPool descriptor_pool;
    VkResult result = vkCreateDescriptorPool(Device::get(), &descriptor_pool_create_info, nullptr, &descriptor_pool);

    // TODO: Error handling
    VK_ASSERT(result);

    return descriptor_pool;
}

void
goose::render::destroy_descriptor_pool(VkDescriptorPool descriptor_pool)
{
    vkDestroyDescriptorPool(Device::get(), descriptor_pool, nullptr);
}

void
goose::render::destroy_descriptor_set_layout(VkDescriptorSetLayout descriptor_set_layout)
{
    vkDestroyDescriptorSetLayout(Device::get(), descriptor_set_layout, nullptr);
}

VkDescriptorSet
goose::render::allocate_descriptor_set(VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout)
{
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptor_set_layout,
    };

    VkDescriptorSet descriptor_set;
    VkResult result = vkAllocateDescriptorSets(Device::get(), &descriptor_set_allocate_info, &descriptor_set);

    // TODO: Error handling
    VK_ASSERT(result);

    return descriptor_set;
}

VkFence
goose::render::create_fence(VkFenceCreateFlags fence_create_flags)
{
    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = fence_create_flags,
    };

    VkFence fence;
    VkResult result = vkCreateFence(Device::get(), &fence_create_info, nullptr, &fence);

    // TODO: Error handling
    VK_ASSERT(result);

    return fence;
}

VkSemaphore
goose::render::create_semaphore(VkSemaphoreCreateFlags semaphore_create_flags)
{
    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .flags = semaphore_create_flags,
    };

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(Device::get(), &semaphore_create_info, nullptr, &semaphore);

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

VkImageCreateInfo
goose::render::make_image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
{
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage_flags,
    };
}

VkImageViewCreateInfo
goose::render::make_image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags)
{
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
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

void
goose::render::copy_image_to_image(
    VkCommandBuffer command_buffer,
    VkImage src_image,
    VkImage dst_image,
    VkExtent2D src_extent,
    VkExtent2D dst_extent)
{
    VkImageBlit2 blit_region = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
    };

    blit_region.srcOffsets[1].x = src_extent.width;
    blit_region.srcOffsets[1].y = src_extent.height;
    blit_region.srcOffsets[1].z = 1;

    blit_region.dstOffsets[1].x = dst_extent.width;
    blit_region.dstOffsets[1].y = dst_extent.height;
    blit_region.dstOffsets[1].z = 1;

    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;

    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blit_info = {
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .srcImage = src_image,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = dst_image,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1,
        .pRegions = &blit_region,
        .filter = VK_FILTER_LINEAR,
    };

    vkCmdBlitImage2(command_buffer, &blit_info);
}

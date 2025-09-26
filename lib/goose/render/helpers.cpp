#include "goose/render/helpers.hpp"

#include "goose/common/assert.hpp"
#include "goose/common/log.hpp"
#include "goose/common/util.hpp"
#include "goose/render/device.hpp"

VkCommandPool
goose::render::create_command_pool(u32 queue_family_index, VkCommandPoolCreateFlags create_flags)
{
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = create_flags,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool command_pool;
    VkResult result = vkCreateCommandPool(Device::get(), &command_pool_create_info, nullptr, &command_pool);

    // TODO: Error handling
    VK_ASSERT(result);

    return command_pool;
}

void
goose::render::destroy_command_pool(VkCommandPool pool)
{
    vkDestroyCommandPool(Device::get(), pool, nullptr);
}

VkCommandBuffer
goose::render::allocate_command_buffer(VkCommandPool pool, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = level,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(Device::get(), &command_buffer_alloc_info, &command_buffer);

    // TODO: Error handling
    VK_ASSERT(result);

    return command_buffer;
}

VkDescriptorPool
goose::render::create_descriptor_pool(u32 max_sets, std::span<VkDescriptorPoolSize> pool_sizes)
{
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = max_sets,
        .poolSizeCount = static_cast<u32>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    VkDescriptorPool descriptor_pool;
    VkResult result = vkCreateDescriptorPool(Device::get(), &descriptor_pool_create_info, nullptr, &descriptor_pool);

    // TODO: Error handling
    VK_ASSERT(result);

    return descriptor_pool;
}

void
goose::render::destroy_descriptor_pool(VkDescriptorPool pool)
{
    vkDestroyDescriptorPool(Device::get(), pool, nullptr);
}

VkDescriptorSet
goose::render::allocate_descriptor_set(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet descriptor_set;
    VkResult result = vkAllocateDescriptorSets(Device::get(), &descriptor_set_allocate_info, &descriptor_set);

    // TODO: Error handling
    VK_ASSERT(result);

    return descriptor_set;
}

VkFence
goose::render::create_fence(VkFenceCreateFlags create_flags)
{
    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = create_flags,
    };

    VkFence fence;
    VkResult result = vkCreateFence(Device::get(), &fence_create_info, nullptr, &fence);

    // TODO: Error handling
    VK_ASSERT(result);

    return fence;
}

void
goose::render::destroy_fence(VkFence fence)
{
    vkDestroyFence(Device::get(), fence, nullptr);
}

VkSemaphore
goose::render::create_semaphore(VkSemaphoreCreateFlags create_flags)
{
    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .flags = create_flags,
    };

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(Device::get(), &semaphore_create_info, nullptr, &semaphore);

    // TODO: Error handling
    VK_ASSERT(result);

    return semaphore;
}

void
goose::render::destroy_semaphore(VkSemaphore semaphore)
{
    vkDestroySemaphore(Device::get(), semaphore, nullptr);
}

VkShaderModule
goose::render::create_shader_module(const std::string &file_path)
{
    std::vector<u32> shader;
    if (!read_file(shader, file_path))
    {
        LOG_ERROR("Failed to read shader file: {}", file_path);
        return nullptr;
    }

    VkShaderModuleCreateInfo shader_module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shader.size() * sizeof(u32),
        .pCode = shader.data(),
    };

    VkShaderModule shader_module;
    VkResult result = vkCreateShaderModule(Device::get(), &shader_module_create_info, nullptr, &shader_module);

    // TODO: Error handling
    VK_ASSERT(result);

    return shader_module;
}

void
goose::render::destroy_shader_module(VkShaderModule shader_module)
{
    vkDestroyShaderModule(Device::get(), shader_module, nullptr);
}

void
goose::render::transition_image(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_flags =
        new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
            ? VK_IMAGE_ASPECT_DEPTH_BIT
            : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange subresource_range = {
        .aspectMask = aspect_flags,
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

    vkCmdPipelineBarrier2(cmd, &dependency_info);
}

void
goose::render::copy_image_to_image(
    VkCommandBuffer cmd,
    VkImage src_image,
    VkImage dst_image,
    VkExtent3D src_extent,
    VkExtent3D dst_extent)
{
    VkImageBlit2 blit_region = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
    };

    blit_region.srcOffsets[1].x = src_extent.width;
    blit_region.srcOffsets[1].y = src_extent.height;
    blit_region.srcOffsets[1].z = src_extent.depth;

    blit_region.dstOffsets[1].x = dst_extent.width;
    blit_region.dstOffsets[1].y = dst_extent.height;
    blit_region.dstOffsets[1].z = dst_extent.depth;

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

    vkCmdBlitImage2(cmd, &blit_info);
}

void
goose::render::copy_image_to_image(
    VkCommandBuffer cmd,
    VkImage src_image,
    VkImage dst_image,
    VkExtent2D src_extent,
    VkExtent2D dst_extent)
{
    copy_image_to_image(cmd, src_image, dst_image, {src_extent.width, src_extent.height, 1}, {dst_extent.width, dst_extent.height, 1});
}

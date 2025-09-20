#include "goose/render/context.hpp"

#include "goose/common/util.hpp"
#include "goose/render/image.hpp"
#include "goose/window/window.hpp"

bool
goose::render::create_render_context(const Window &window, RenderContext &ctx)
{
    // TODO: Allow usage without surface
    if (window.surface == VK_NULL_HANDLE)
    {
        LOG_ERROR("Missing surface");
        return false;
    }

    if (!create_device(window.surface, ctx.device))
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    if (!create_swapchain(ctx.device, window.surface, window.extent, ctx.swapchain))
    {
        LOG_ERROR("Failed to create swapchain");
        return false;
    }

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (!create_frame(ctx.device, ctx.frames[i]))
        {
            LOG_ERROR("Failed to create frame data for frame {}", i);
            return false;
        }
    }

    ctx.current_frame = 0;

    return true;
}

void
goose::render::destroy_render_context(RenderContext &ctx)
{
    vkDeviceWaitIdle(ctx.device.logical);

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        destroy_frame(ctx.device, ctx.frames[i]);
    }

    destroy_swapchain(ctx.device, ctx.swapchain);
    destroy_device(ctx.device);
}

void
goose::render::begin_frame(RenderContext &ctx)
{
    Frame &frame = ctx.frames[ctx.current_frame];

    vkWaitForFences(ctx.device.logical, 1, &frame.in_flight_fence, true, 1000000000);
    vkResetFences(ctx.device.logical, 1, &frame.in_flight_fence);

    u32 swapchain_image_index;
    vkAcquireNextImageKHR(ctx.device.logical, ctx.swapchain.handle, 1000000000, frame.image_available_semaphore, nullptr, &swapchain_image_index);

    begin_command_buffer(frame);

    transition_image(frame.command_buffer, ctx.swapchain.images[swapchain_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clear_color_value = {{0.0f, 0.0f, 1.0f, 1.0f}};

    VkImageSubresourceRange clear_subresource_range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };

    vkCmdClearColorImage(frame.command_buffer, ctx.swapchain.images[swapchain_image_index], VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1, &clear_subresource_range);

    transition_image(frame.command_buffer, ctx.swapchain.images[swapchain_image_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    end_command_buffer(frame);

    VkCommandBufferSubmitInfo command_buffer_submit_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = frame.command_buffer,
        .deviceMask = 0,
    };

    VkSemaphoreSubmitInfo wait_semaphore_submit_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.image_available_semaphore,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        .deviceIndex = 0,
    };

    VkSemaphoreSubmitInfo signal_semaphore_submit_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.render_finished_semaphore,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        .deviceIndex = 0,
    };

    VkSubmitInfo2 submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_semaphore_submit_info,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_buffer_submit_info,

        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_semaphore_submit_info,
    };

    VkResult result = vkQueueSubmit2(ctx.device.queue_families.graphics.queues[0], 1, &submit_info, frame.in_flight_fence);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("ALARM!!!");
    }

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &ctx.swapchain.handle,
        .pImageIndices = &swapchain_image_index,
    };

    result = vkQueuePresentKHR(ctx.device.queue_families.graphics.queues[0], &present_info);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("ALAAAAARM!!!");
    }

    ctx.current_frame = (ctx.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
goose::render::end_frame(RenderContext &ctx)
{
}

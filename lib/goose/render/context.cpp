#include "goose/render/context.hpp"

#include "goose/common/log.hpp"
#include "goose/render/util.hpp"
#include "goose/window/window.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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

std::pair<VkCommandBuffer, VkImage>
goose::render::begin_frame(RenderContext &ctx)
{
    Frame &frame = ctx.frames[ctx.current_frame];

    vkWaitForFences(ctx.device.logical, 1, &frame.in_flight_fence, true, 1000000000);
    vkResetFences(ctx.device.logical, 1, &frame.in_flight_fence);

    vkAcquireNextImageKHR(ctx.device.logical, ctx.swapchain.handle, 1000000000, frame.image_available_semaphore, nullptr, &ctx.current_swapchain_image);

    SwapchainImage &swapchain_image = ctx.swapchain.images[ctx.current_swapchain_image];

    begin_command_buffer(frame);
    transition_image(frame.command_buffer, swapchain_image.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    return {frame.command_buffer, swapchain_image.handle};
}

void
goose::render::end_frame(RenderContext &ctx)
{
    Frame &frame = ctx.frames[ctx.current_frame];
    SwapchainImage &swapchain_image = ctx.swapchain.images[ctx.current_swapchain_image];

    transition_image(frame.command_buffer, swapchain_image.handle, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    end_command_buffer(frame);

    VkSemaphoreSubmitInfo wait_semaphore_submit_info =
        make_semaphore_submit_info(frame.image_available_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);

    VkCommandBufferSubmitInfo command_buffer_submit_info =
        make_command_buffer_submit_info(frame.command_buffer);

    VkSemaphoreSubmitInfo signal_semaphore_submit_info =
        make_semaphore_submit_info(swapchain_image.render_finished_semaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

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

    // TODO: Error handling
    VK_ASSERT(result);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain_image.render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &ctx.swapchain.handle,
        .pImageIndices = &ctx.current_swapchain_image,
    };

    result = vkQueuePresentKHR(ctx.device.queue_families.graphics.queues[0], &present_info);

    // TODO: Error handling
    VK_ASSERT(result);

    ctx.current_frame = (ctx.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

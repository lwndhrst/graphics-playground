#include "goose/render/context.hpp"

#include "goose/common/log.hpp"
#include "goose/render/allocator.hpp"
#include "goose/render/device.hpp"
#include "goose/render/util.hpp"
#include "goose/window/window.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

bool
goose::render::create_render_context(RenderContext &ctx, const Window &window)
{
    // TODO: Allow usage without surface
    if (window.surface == VK_NULL_HANDLE)
    {
        LOG_ERROR("Missing surface");
        return false;
    }

    // NOTE: The device is a singleton and will only be created once
    //       It's created here instead of in goose::init() as it needs a valid surface
    if (!create_device(window.surface))
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    // NOTE: The allocator is a singleton and will only be created once
    //       It's created here instead of in goose::init() as it needs a valid device
    if (!create_allocator())
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    if (!create_swapchain(ctx.swapchain, window.surface, window.extent))
    {
        LOG_ERROR("Failed to create swapchain");
        return false;
    }

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (!create_frame(ctx.frames[i]))
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
    const Device &device = get_device();
    vkDeviceWaitIdle(device.logical);

    for (const auto &f : ctx.cleanup_callbacks)
    {
        f();
    }

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        destroy_frame(ctx.frames[i]);
    }

    destroy_swapchain(ctx.swapchain);

    // NOTE: The allocator singleton is destroyed when goose::quit() is called
    // NOTE: The device singleton is destroyed when goose::quit() is called

    ctx = {};
}

void
goose::render::add_cleanup_callback(RenderContext &ctx, const std::function<void()> &&callback)
{
    ctx.cleanup_callbacks.emplace_back(callback);
}

void
goose::render::resize_swapchain(RenderContext &ctx, const Window &window)
{
    const Device &device = get_device();
    vkDeviceWaitIdle(device.logical);

    destroy_swapchain(ctx.swapchain);
    create_swapchain(ctx.swapchain, window.surface, window.extent);
}

std::pair<VkCommandBuffer, const goose::render::SwapchainImage &>
goose::render::begin_frame(RenderContext &ctx)
{
    const Device &device = get_device();
    Frame &frame = ctx.frames[ctx.current_frame];

    vkWaitForFences(device.logical, 1, &frame.in_flight_fence, true, 1000000000);
    vkResetFences(device.logical, 1, &frame.in_flight_fence);

    vkAcquireNextImageKHR(device.logical, ctx.swapchain.swapchain, 1000000000, frame.image_available_semaphore, nullptr, &ctx.current_swapchain_image);

    const SwapchainImage &swapchain_image = ctx.swapchain.images[ctx.current_swapchain_image];

    begin_command_buffer(frame);

    return {frame.command_buffer, swapchain_image};
}

void
goose::render::end_frame(RenderContext &ctx)
{
    const Device &device = get_device();
    Frame &frame = ctx.frames[ctx.current_frame];

    const SwapchainImage &swapchain_image = ctx.swapchain.images[ctx.current_swapchain_image];

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

    VkResult result = vkQueueSubmit2(device.queue_families.graphics.queues[0], 1, &submit_info, frame.in_flight_fence);

    // TODO: Error handling
    VK_ASSERT(result);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain_image.render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &ctx.swapchain.swapchain,
        .pImageIndices = &ctx.current_swapchain_image,
    };

    result = vkQueuePresentKHR(device.queue_families.graphics.queues[0], &present_info);

    // TODO: Error handling
    VK_ASSERT(result);

    ctx.current_frame = (ctx.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

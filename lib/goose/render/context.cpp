#include "goose/render/context.hpp"

#include "goose/common/log.hpp"
#include "goose/render/allocator.hpp"
#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"
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
        if (!create_frame_data(ctx.frames[i]))
        {
            LOG_ERROR("Failed to create frame data for frame {}", i);
            return false;
        }
    }

    if (!create_immediate_data(ctx.immediate))
    {
        LOG_ERROR("Failed to create immediate submit data");
        return false;
    }

    ctx.current_frame = 0;

    return true;
}

void
goose::render::destroy_render_context(RenderContext &ctx)
{
    vkDeviceWaitIdle(Device::get());

    // Call cleanup callbacks in reverse order of being added
    for (auto f = ctx.cleanup_callbacks.rbegin(); f != ctx.cleanup_callbacks.rend(); ++f)
    {
        (*f)();
    }

    destroy_immediate_data(ctx.immediate);

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        destroy_frame_data(ctx.frames[i]);
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
    vkDeviceWaitIdle(Device::get());

    destroy_swapchain(ctx.swapchain);
    create_swapchain(ctx.swapchain, window.surface, window.extent);
}

std::pair<VkCommandBuffer, const goose::render::SwapchainImage &>
goose::render::begin_frame(RenderContext &ctx)
{
    // TODO: Error handling

    const VkDevice &device = Device::get();
    const FrameData &frame = ctx.frames[ctx.current_frame];

    vkWaitForFences(device, 1, &frame.in_flight_fence, true, 1000000000);
    vkResetFences(device, 1, &frame.in_flight_fence);

    vkAcquireNextImageKHR(device, ctx.swapchain.swapchain, 1000000000, frame.image_available_semaphore, nullptr, &ctx.current_swapchain_image);

    const SwapchainImage &swapchain_image = ctx.swapchain.images[ctx.current_swapchain_image];

    vkResetCommandBuffer(frame.command_buffer, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(frame.command_buffer, &command_buffer_begin_info);

    return {frame.command_buffer, swapchain_image};
}

void
goose::render::end_frame(RenderContext &ctx)
{
    // TODO: Error handling

    const FrameData &frame = ctx.frames[ctx.current_frame];
    const SwapchainImage &swapchain_image = ctx.swapchain.images[ctx.current_swapchain_image];

    vkEndCommandBuffer(frame.command_buffer);

    VkCommandBufferSubmitInfo command_buffer_submit_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = frame.command_buffer,
    };

    VkSemaphoreSubmitInfo wait_semaphore_submit_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.image_available_semaphore,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
    };

    VkSemaphoreSubmitInfo signal_semaphore_submit_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = swapchain_image.render_finished_semaphore,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
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

    const QueueFamilies &queue_families = Device::get_queue_families();

    VkResult result = vkQueueSubmit2(queue_families.graphics.queues[0], 1, &submit_info, frame.in_flight_fence);

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

    result = vkQueuePresentKHR(queue_families.graphics.queues[0], &present_info);

    // TODO: Error handling
    VK_ASSERT(result);

    ctx.current_frame = (ctx.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer
goose::render::begin_immediate(const RenderContext &ctx)
{
    // TODO: Error handling

    const VkDevice &device = Device::get();
    const ImmediateData &immediate = ctx.immediate;

    vkResetFences(device, 1, &immediate.in_flight_fence);

    vkResetCommandBuffer(immediate.command_buffer, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(immediate.command_buffer, &command_buffer_begin_info);

    return immediate.command_buffer;
}

void
goose::render::end_immediate(const RenderContext &ctx)
{
    // TODO: Error handling

    const VkDevice &device = Device::get();
    const ImmediateData &immediate = ctx.immediate;

    vkEndCommandBuffer(immediate.command_buffer);

    VkCommandBufferSubmitInfo command_buffer_submit_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = immediate.command_buffer,
    };

    VkSubmitInfo2 submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_buffer_submit_info,
    };

    const QueueFamilies &queue_families = Device::get_queue_families();

    VkResult result = vkQueueSubmit2(queue_families.graphics.queues[0], 1, &submit_info, immediate.in_flight_fence);

    // TODO: Error handling
    VK_ASSERT(result);

    vkWaitForFences(device, 1, &immediate.in_flight_fence, true, 9999999999);
}

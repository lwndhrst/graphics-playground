#include "goose/render/context.hpp"

#include "goose/common/util.hpp"
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
goose::render::begin_drawing(RenderContext &ctx)
{
}

void
goose::render::end_drawing(RenderContext &ctx)
{
    ctx.current_frame = (ctx.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

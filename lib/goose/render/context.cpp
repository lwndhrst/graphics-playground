#include "goose/render/context.hpp"

#include "goose/common/util.hpp"
#include "goose/render/instance.hpp"
#include "goose/window/window.hpp"

bool
goose::render::create_render_context(const goose::window::Window &window, RenderContext &ctx)
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

    return true;
}

void
goose::render::destroy_render_context(RenderContext &ctx)
{
    vkDeviceWaitIdle(ctx.device.logical);

    destroy_swapchain(ctx.device, ctx.swapchain);
    destroy_device(ctx.device);
}

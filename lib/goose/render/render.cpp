#include "goose/render/render.hpp"

#include "goose/core/util.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"
#include "goose/render/swapchain.hpp"

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

bool
goose::render::create_instance(RenderContext *ctx, const char *app_name, u32 app_version)
{
#ifdef GOOSE_DEBUG
    ctx->instance.layers.push_back(VALIDATION_LAYER_NAME);
    ctx->instance.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    ctx->instance.extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ctx->instance.extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    ctx->instance.handle = create_instance(
        app_name,
        app_version,
        ctx->instance.layers,
        ctx->instance.extensions);

    return ctx->instance.handle != VK_NULL_HANDLE;
}

bool
goose::render::init(RenderContext *ctx)
{
    if (ctx->window.surface == VK_NULL_HANDLE)
    {
        LOG_ERROR("Missing surface");
        return false;
    }

    // TODO: Which device extensions are required?
    ctx->device.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    ctx->device.physical = get_gpu(
        ctx->instance.handle,
        ctx->window.surface,
        ctx->device.extensions);

    if (ctx->device.physical == VK_NULL_HANDLE)
    {
        LOG_ERROR("No suitable GPU found");
        return false;
    }

#ifdef GOOSE_DEBUG
    // NOTE: Modern Vulkan doesn't seem to distinguish between instance and device layers anymore.
    //       This will just be ignored on modern Vulkan versions, but leaving this here anyway.
    ctx->device.layers.push_back(VALIDATION_LAYER_NAME);
#endif

    const auto &[device, device_queues] = create_logical_device(
        ctx->device.physical,
        ctx->window.surface,
        ctx->device.layers,
        ctx->device.extensions);

    if (device == VK_NULL_HANDLE)
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    ctx->device.logical = device;
    ctx->queues.graphics = device_queues.graphics;
    ctx->queues.present = device_queues.present;

    ctx->swapchain.handle = create_swapchain(
        ctx->device.logical,
        ctx->device.physical,
        ctx->window.surface,
        ctx->window.handle);

    if (ctx->swapchain.handle == VK_NULL_HANDLE)
    {
        LOG_ERROR("Failed to create swapchain");
        return false;
    }

    return true;
}

void
goose::render::cleanup(RenderContext *ctx)
{
    vkDeviceWaitIdle(ctx->device.logical);

    vkDestroySwapchainKHR(ctx->device.logical, ctx->swapchain.handle, nullptr);
    vkDestroyDevice(ctx->device.logical, nullptr);
    vkDestroySurfaceKHR(ctx->instance.handle, ctx->window.surface, nullptr);
    vkDestroyInstance(ctx->instance.handle, nullptr);
}

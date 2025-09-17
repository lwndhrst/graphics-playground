#include "goose/render/render.hpp"

#include "goose/core/util.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"
#include "goose/render/swapchain.hpp"

#include "SDL3/SDL_vulkan.h"

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

bool
goose::render::create_instance(RenderContext *ctx, const char *app_name, u32 app_version)
{
    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;

#ifdef GOOSE_DEBUG
    instance_layers.push_back(VALIDATION_LAYER_NAME);
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    u32 sdl_instance_extension_count;
    auto sdl_instance_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_instance_extension_count);
    for (usize i = 0; i < sdl_instance_extension_count; ++i)
    {
        instance_extensions.push_back(sdl_instance_extensions[i]);
    }

    instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance_extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    ctx->instance = create_instance(
        app_name,
        app_version,
        instance_layers,
        instance_extensions);

    return ctx->instance.handle != VK_NULL_HANDLE;
}

bool
goose::render::init(RenderContext *ctx, VkExtent2D window_extent, VkSurfaceKHR surface)
{
    if (surface == VK_NULL_HANDLE)
    {
        LOG_ERROR("Missing surface");
        return false;
    }

    ctx->window = {
        .extent = window_extent,
        .surface = surface,
    };

    std::vector<const char *> device_layers;
    std::vector<const char *> device_extensions;

#ifdef GOOSE_DEBUG
    // NOTE: Modern Vulkan doesn't seem to distinguish between instance and device layers anymore.
    //       This will just be ignored on modern Vulkan versions, but leaving this here anyway.
    device_layers.push_back(VALIDATION_LAYER_NAME);
#endif

    // TODO: Which device extensions are required?
    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    ctx->device = create_device(
        ctx->instance.handle,
        ctx->window.surface,
        device_layers,
        device_extensions);

    if (ctx->device.logical == VK_NULL_HANDLE)
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    ctx->swapchain = create_swapchain(
        ctx->device.logical,
        ctx->device.physical,
        ctx->window.surface,
        ctx->window.extent);

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

#include "goose/render/render.hpp"

#include "goose/core/util.hpp"
#include "goose/render/device.hpp"

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

namespace goose::render {

bool
create_instance(RenderContext *ctx, const char *app_name, u32 app_version)
{
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name,
        .applicationVersion = app_version,
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    // TODO: Properly check layer/extension support

#ifdef GOOSE_DEBUG
    ctx->instance.layers.push_back(VALIDATION_LAYER_NAME);
    ctx->instance.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<u32>(ctx->instance.layers.size()),
        .ppEnabledLayerNames = ctx->instance.layers.data(),
        .enabledExtensionCount = static_cast<u32>(ctx->instance.extensions.size()),
        .ppEnabledExtensionNames = ctx->instance.extensions.data(),
    };

    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &ctx->instance.handle);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);
        return false;
    }

    return true;
}

bool
init(RenderContext *ctx)
{
    if (ctx->window.surface == nullptr)
    {
        LOG_ERROR("Missing surface");
        return false;
    }

    // TODO: Which device extensions are required?
    ctx->device.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkPhysicalDevice gpu = get_gpu(ctx->instance.handle, ctx->window.surface, ctx->device.extensions);
    if (gpu == nullptr)
    {
        LOG_ERROR("No suitable GPU found");
        return false;
    }

#ifdef GOOSE_DEBUG
    // NOTE: Modern Vulkan doesn't seem to distinguish between instance and device layers anymore.
    //       This will just be ignored on modern Vulkan versions, but leaving this here anyway.
    ctx->device.layers.push_back(VALIDATION_LAYER_NAME);
#endif

    ctx->device.handle = create_logical_device(gpu, ctx->window.surface, ctx->device.layers, ctx->device.extensions, ctx->queues);
    if (ctx->device.handle == nullptr)
    {
        LOG_ERROR("Failed to create logical device");
        return false;
    }

    return true;
}

void
cleanup(RenderContext *ctx)
{
    vkDeviceWaitIdle(ctx->device.handle);

    vkDestroyDevice(ctx->device.handle, nullptr);
    vkDestroySurfaceKHR(ctx->instance.handle, ctx->window.surface, nullptr);
    vkDestroyInstance(ctx->instance.handle, nullptr);
}

} // namespace goose::render

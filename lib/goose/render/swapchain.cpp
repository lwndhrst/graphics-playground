#include "goose/render/swapchain.hpp"

#include "goose/core/util.hpp"
#include "goose/render/device.hpp"

#include "SDL3/SDL_video.h"

goose::render::SwapchainSupportDetails
goose::render::get_swapchain_support_details(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details = {};

    VkPhysicalDeviceSurfaceInfo2KHR surface_info = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .surface = surface,
    };

    VkSurfaceCapabilities2KHR surface_capabilities = {
        .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
    };

    vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu, &surface_info, &surface_capabilities);

    details.capabilities = surface_capabilities.surfaceCapabilities;

    u32 surface_format_count;
    vkGetPhysicalDeviceSurfaceFormats2KHR(gpu, &surface_info, &surface_format_count, nullptr);

    VkSurfaceFormat2KHR surface_format = {
        .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
    };

    std::vector<VkSurfaceFormat2KHR> surface_formats(surface_format_count, surface_format);
    vkGetPhysicalDeviceSurfaceFormats2KHR(gpu, &surface_info, &surface_format_count, surface_formats.data());

    details.formats.resize(surface_format_count);
    for (const auto &format : surface_formats)
    {
        details.formats.push_back(format.surfaceFormat);
    }

    u32 surface_present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &surface_present_mode_count, nullptr);

    details.present_modes.resize(surface_present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &surface_present_mode_count, details.present_modes.data());

    return details;
}

static VkSurfaceFormatKHR
choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats)
{
    for (const auto &format : available_formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    // Default to first available format in the list
    return available_formats[0];
}

static VkPresentModeKHR
choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes)
{
    for (const auto &availablePresentMode : available_present_modes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    // Default to FIFO since it's guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D
choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
    {
        return capabilities.currentExtent;
    }

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    VkExtent2D extent = {
        .width = std::clamp<u32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        .height = std::clamp<u32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };

    return extent;
}

VkSwapchainKHR
goose::render::create_swapchain(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, SDL_Window *window)
{
    SwapchainSupportDetails swapchain_support = get_swapchain_support_details(gpu, surface);

    VkSurfaceFormatKHR surface_format = choose_swapchain_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
    VkExtent2D extent = choose_swapchain_extent(swapchain_support.capabilities, window);

    u32 image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0)
    {
        image_count = std::max(image_count, swapchain_support.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapchain_support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    QueueFamilyIndices indices = get_queue_family_indices(gpu, surface);
    u32 queue_family_indices[] = {
        indices.graphics.value(),
        indices.present.value(),
    };

    if (indices.graphics != indices.present)
    {
        // TODO: Is VK_SHARING_MODE_CONCURRENT the way?
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        // NOTE: Graphics and present indices should be the same on most hardware
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);

        // TODO: Error handling
    }

    return swapchain;
}

#include "goose/render/swapchain.hpp"

#include "goose/common/util.hpp"
#include "goose/render/device.hpp"

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
choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D window_extent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
    {
        return capabilities.currentExtent;
    }

    return {
        .width = std::clamp<u32>(window_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        .height = std::clamp<u32>(window_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };
}

bool
goose::render::create_swapchain(
    const Device &device,
    VkSurfaceKHR surface,
    VkExtent2D window_extent,
    Swapchain &swapchain)
{
    // Create swapchain

    SwapchainSupportDetails swapchain_support = get_swapchain_support_details(device.physical, surface);

    VkSurfaceFormatKHR swapchain_surface_format = choose_swapchain_surface_format(swapchain_support.formats);
    VkPresentModeKHR swapchain_present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
    VkExtent2D swapchain_extent = choose_swapchain_extent(swapchain_support.capabilities, window_extent);

    u32 swapchain_image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0)
    {
        swapchain_image_count = std::min(swapchain_image_count, swapchain_support.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = swapchain_image_count,
        .imageFormat = swapchain_surface_format.format,
        .imageColorSpace = swapchain_surface_format.colorSpace,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = swapchain_support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapchain_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    // TODO: How do deal with compute queue here?
    if (device.queue_families.graphics.index != device.queue_families.present.index)
    {
        u32 queue_family_indices[] = {
            device.queue_families.graphics.index,
            device.queue_families.present.index,
        };

        // TODO: Is VK_SHARING_MODE_CONCURRENT the way?
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        // NOTE: Graphics and present queue indices should be the same on most hardware
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    VkResult result = vkCreateSwapchainKHR(device.logical, &swapchain_create_info, nullptr, &swapchain.handle);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);
        return false;
    }

    // Get swapchain images

    vkGetSwapchainImagesKHR(device.logical, swapchain.handle, &swapchain_image_count, nullptr);

    swapchain.images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(device.logical, swapchain.handle, &swapchain_image_count, swapchain.images.data());

    // Create swapchain image views

    VkImageViewCreateInfo image_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain_surface_format.format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    swapchain.image_views.resize(swapchain_image_count);
    for (usize i = 0; i < swapchain_image_count; ++i)
    {
        image_view_create_info.image = swapchain.images[i];

        VkResult result = vkCreateImageView(device.logical, &image_view_create_info, nullptr, &swapchain.image_views[i]);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result);
            return false;
        }
    }

    swapchain.extent = swapchain_extent;
    swapchain.format = swapchain_surface_format.format;

    return true;
}

void
goose::render::destroy_swapchain(const Device &device, Swapchain &swapchain)
{
    for (auto image_view : swapchain.image_views)
    {
        vkDestroyImageView(device.logical, image_view, nullptr);
    }

    vkDestroySwapchainKHR(device.logical, swapchain.handle, nullptr);
}

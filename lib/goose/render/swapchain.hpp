#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails get_swapchain_support_details(VkPhysicalDevice gpu, VkSurfaceKHR surface);

struct SwapchainImageInfo {
    VkImage image;
    VkImageView view;

    // Identical to swapchain extent and format
    VkExtent2D extent;
    VkFormat format;

    // Signaled when the image is ready for presentation, i.e. when all submitted work is done
    VkSemaphore render_finished_semaphore;
};

struct SwapchainInfo {
    VkSwapchainKHR swapchain;

    VkExtent2D image_extent;
    VkFormat image_format;

    u32 image_count;
    std::vector<SwapchainImageInfo> images;
};

bool create_swapchain(SwapchainInfo &swapchain, VkSurfaceKHR surface, VkExtent2D window_extent);
void destroy_swapchain(SwapchainInfo &swapchain);

} // namespace goose::render

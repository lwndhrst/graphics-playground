#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails get_swapchain_support_details(VkPhysicalDevice gpu, VkSurfaceKHR surface);

struct SwapchainImage {
    VkImage image;
    VkImageView view;

    // Identical to swapchain extent and format
    VkExtent2D extent;
    VkFormat format;

    // Signaled when the image is ready for presentation, i.e. when rendering is finished
    VkSemaphore render_finished_semaphore;
};

struct Swapchain {
    VkSwapchainKHR swapchain;

    VkExtent2D image_extent;
    VkFormat image_format;

    usize image_count;
    std::vector<SwapchainImage> images;
};

bool create_swapchain(Swapchain &swapchain, VkSurfaceKHR surface, VkExtent2D window_extent);
void destroy_swapchain(Swapchain &swapchain);

} // namespace goose::render

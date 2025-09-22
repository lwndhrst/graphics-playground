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
    VkImage handle;
    VkImageView view;

    // Used to wait for the image to be ready for presentation, i.e. for rendering to be finished
    VkSemaphore render_finished_semaphore;
};

struct Swapchain {
    VkSwapchainKHR handle;
    VkExtent2D extent;
    VkFormat format;
    usize image_count;
    std::vector<SwapchainImage> images;
};

bool create_swapchain(VkSurfaceKHR surface, VkExtent2D window_extent, Swapchain &swapchain);
void destroy_swapchain(Swapchain &swapchain);

} // namespace goose::render

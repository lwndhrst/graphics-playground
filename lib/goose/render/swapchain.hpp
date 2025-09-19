#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails get_swapchain_support_details(VkPhysicalDevice gpu, VkSurfaceKHR surface);

struct Swapchain {
    VkSwapchainKHR handle;
    VkExtent2D extent;
    VkFormat format;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
};

bool create_swapchain(const Device &device, VkSurfaceKHR surface, VkExtent2D window_extent, Swapchain &swapchain);
void destroy_swapchain(const Device &device, Swapchain &swapchain);

} // namespace goose::render

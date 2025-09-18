#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct Device;

struct Swapchain {
    VkSwapchainKHR handle;
    VkExtent2D extent;
    VkFormat format;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails get_swapchain_support_details(VkPhysicalDevice gpu, VkSurfaceKHR surface);

Swapchain create_swapchain(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkExtent2D window_extent);

void destroy_swapchain(Device &device, Swapchain &swapchain);

} // namespace goose::render

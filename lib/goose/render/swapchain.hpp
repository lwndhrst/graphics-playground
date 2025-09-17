#pragma once

#include "goose/core/types.hpp"

struct SDL_Window;

namespace goose::render {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails get_swapchain_support_details(VkPhysicalDevice gpu, VkSurfaceKHR surface);

VkSwapchainKHR create_swapchain(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, SDL_Window *window);

} // namespace goose::render

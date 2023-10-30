#pragma once

#include "core.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <optional>
#include <vulkan/vulkan.h>

namespace gp {

struct DeviceQueueFamilyIndices {
    std::optional<u32> graphics;
    std::optional<u32> present;
};

struct DeviceQueues {
    VkQueue graphics;
    VkQueue present;
};

struct DeviceSwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    u32 surface_format_count;
    u32 present_mode_count;
    VkSurfaceFormatKHR *surface_formats;
    VkPresentModeKHR *present_modes;

    DeviceSwapchainSupportDetails(u32 surface_format_count, u32 present_mode_count);
    ~DeviceSwapchainSupportDetails();
};

struct RendererData {
    SDL_Window *window;
    VkSurfaceKHR surface;

    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;

    DeviceQueueFamilyIndices queue_family_indices;
    DeviceQueues queues;
};

class Renderer {
private:
    RendererData data;

public:
    bool init(SDL_Window *window);
    void draw();
    void cleanup();
};

} // namespace gp

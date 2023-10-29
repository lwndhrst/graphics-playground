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
    VkSurfaceFormatKHR *surface_formats;
    u32 present_mode_count;
    VkPresentModeKHR *present_modes;

    DeviceSwapchainSupportDetails(u32 surface_format_count,
                                  u32 present_mode_count)
        : surface_format_count(surface_format_count),
          present_mode_count(present_mode_count)
    {
        surface_formats = (VkSurfaceFormatKHR *)malloc(surface_format_count / sizeof(uintptr_t));
        present_modes = (VkPresentModeKHR *)malloc(present_mode_count / sizeof(uintptr_t));
    }

    ~DeviceSwapchainSupportDetails()
    {
        free(surface_formats);
        free(present_modes);
    }
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

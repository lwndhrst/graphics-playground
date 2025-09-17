#pragma once

#include "goose/core/types.hpp"

struct SDL_Window;

namespace goose::render {

struct RenderContext {
    struct Window {
        SDL_Window *handle;
        VkExtent2D extent;
        VkSurfaceKHR surface;
    };

    struct Instance {
        VkInstance handle;
        std::vector<const char *> layers;
        std::vector<const char *> extensions;
    };

    struct Device {
        VkPhysicalDevice physical;
        VkDevice logical;
        std::vector<const char *> layers;
        std::vector<const char *> extensions;
    };

    struct Queues {
        VkQueue graphics;
        VkQueue present;
    };

    struct Swapchain {
        VkSwapchainKHR handle;
        VkExtent2D extent;
    };

    Window window;
    Instance instance;
    Device device;
    Queues queues;
    Swapchain swapchain;
};

bool create_instance(RenderContext *ctx, const char *app_name, u32 app_version);

bool init(RenderContext *ctx);

void cleanup(RenderContext *ctx);

} // namespace goose::render

#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

struct RenderContext {
    struct Window {
        VkExtent2D extent;
        VkSurfaceKHR surface;
    };

    struct Instance {
        VkInstance handle;
        std::vector<const char *> layers;
        std::vector<const char *> extensions;
        VkDebugUtilsMessengerEXT debug_messenger;
    };

    struct Device {
        VkDevice handle;
        std::vector<const char *> layers;
        std::vector<const char *> extensions;
    };

    struct Queues {
        VkQueue graphics;
        VkQueue present;
    };

    Window window;
    Instance instance;
    Device device;
    Queues queues;
};

bool create_instance(RenderContext *ctx, const char *app_name, u32 app_version);

bool init(RenderContext *ctx);

void cleanup(RenderContext *ctx);

} // namespace goose::render

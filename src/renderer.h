#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

namespace gp {

struct RendererData {
    SDL_Window *window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
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

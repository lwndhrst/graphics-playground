#include "goose/window/window.hpp"

#include "goose/common/util.hpp"
#include "goose/render/instance.hpp"

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"

bool
goose::create_window(const char *title, u32 width, u32 height, Window &window)
{
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN;

    window.handle = SDL_CreateWindow(title, width, height, window_flags);
    if (window.handle == nullptr)
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    const goose::render::Instance &instance = goose::render::get_instance();
    ASSERT(instance.handle != VK_NULL_HANDLE, "Instance is not initialized");

    if (!SDL_Vulkan_CreateSurface(window.handle, instance.handle, nullptr, &window.surface))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    window.id = SDL_GetWindowID(window.handle);
    window.flags = SDL_GetWindowFlags(window.handle);
    window.should_close = false;
    window.extent = {width, height};

    return true;
}

void
goose::destroy_window(Window &window)
{
    const goose::render::Instance &instance = goose::render::get_instance();
    ASSERT(instance.handle != VK_NULL_HANDLE, "Instance is not initialized");

    vkDestroySurfaceKHR(instance.handle, window.surface, nullptr);
    SDL_DestroyWindow(window.handle);
}

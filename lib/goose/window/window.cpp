#include "goose/window/window.hpp"

#include "goose/common/log.hpp"
#include "goose/render/instance.hpp"

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"

static std::vector<goose::Window *> s_active_windows = {};

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
    window.extent = {width, height};

    // Keep track of active windows internally
    s_active_windows.push_back(&window);

    return true;
}

void
goose::destroy_window(Window &window)
{
    if (window.handle == nullptr)
    {
        LOG_WARN("Window can't be destroyed due to invalid SDL window handle");
        return;
    }

    const goose::render::Instance &instance = goose::render::get_instance();
    ASSERT(instance.handle != VK_NULL_HANDLE, "Instance is not initialized");

    vkDestroySurfaceKHR(instance.handle, window.surface, nullptr);
    SDL_DestroyWindow(window.handle);

    // Remove destroyed window from internal list of active windows and move last entry to the freed position
    usize i;
    get_window_by_id(window.id, i);
    s_active_windows[i] = s_active_windows[s_active_windows.size() - 1];
    s_active_windows.pop_back();
}

goose::Window *
goose::get_window_by_id(SDL_WindowID id)
{
    for (goose::Window *window : s_active_windows)
    {
        if (window->id == id)
        {
            return window;
        }
    }

    ASSERT(false, "Couldn't find window matching the given ID");
    return nullptr;
}

goose::Window *
goose::get_window_by_id(SDL_WindowID id, usize &index)
{
    for (usize i = 0; i < s_active_windows.size(); ++i)
    {
        goose::Window *window = s_active_windows[i];
        if (window->id == id)
        {
            index = i;
            return window;
        }
    }

    ASSERT(false, "Couldn't find window matching the given ID");
    return nullptr;
}

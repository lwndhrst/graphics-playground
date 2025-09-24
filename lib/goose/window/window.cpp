#include "goose/window/window.hpp"

#include "goose/common/assert.hpp"
#include "goose/common/log.hpp"
#include "goose/render/instance.hpp"

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"

static std::vector<goose::Window *> s_active_windows = {};

bool
goose::create_window(Window &window, const char *title, u32 width, u32 height, bool resizable)
{
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN;
    if (resizable)
    {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }

    window.window = SDL_CreateWindow(title, width, height, window_flags);
    if (window.window == nullptr)
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    if (!SDL_Vulkan_CreateSurface(window.window, goose::render::Instance::get(), nullptr, &window.surface))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    window.id = SDL_GetWindowID(window.window);
    window.flags = SDL_GetWindowFlags(window.window);
    window.extent = {width, height};

    // Keep track of active windows internally
    s_active_windows.push_back(&window);

    return true;
}

void
goose::destroy_window(Window &window)
{
    if (window.window == nullptr)
    {
        LOG_WARN("Window can't be destroyed due to invalid SDL window handle");
        return;
    }

    vkDestroySurfaceKHR(goose::render::Instance::get(), window.surface, nullptr);
    SDL_DestroyWindow(window.window);

    // Remove destroyed window from internal list of active windows and move last entry to the freed position
    usize i;
    get_window_by_id(window.id, i);
    s_active_windows[i] = s_active_windows[s_active_windows.size() - 1];
    s_active_windows.pop_back();

    window = {};
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

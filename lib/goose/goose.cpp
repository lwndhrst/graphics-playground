#include "goose/goose.hpp"

#include "goose/core/util.hpp"
#include "goose/render/render.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

namespace goose {

static struct Data {
    const char *app_name;
    u32 app_version;

    SDL_Window *window;
    bool window_should_close;
} data;

static render::RenderData render_data;

bool
init(const char *app_name)
{
    // TODO: Make app version configurable
    data.app_name = app_name;
    data.app_version = VK_MAKE_VERSION(0, 1, 0);

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    return true;
}

bool
create_window(const char *title, u32 width, u32 height)
{
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN;

    data.window = SDL_CreateWindow(
        title,
        width,
        height,
        window_flags);

    if (data.window == nullptr)
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    data.window_should_close = false;

    render_data.window_extent.width = width;
    render_data.window_extent.height = height;

    u32 extension_count;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    for (usize i = 0; i < extension_count; ++i)
    {
        render_data.instance_extensions.push_back(extensions[i]);
    }

    if (!goose::render::create_instance(&render_data, data.app_name, data.app_version))
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(data.window, render_data.instance, nullptr, &surface))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    if (!goose::render::init(&render_data, surface))
    {
        LOG_ERROR("Failed to initialize Vulkan renderer");
        return false;
    }

    return true;
}

bool
window_should_close()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            data.window_should_close = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            fmt::println("Window resized");
            break;
        case SDL_EVENT_WINDOW_OCCLUDED: // NOTE: Interesting for wayland (switching workspaces, etc.)
            fmt::println("Window occluded");
            break;
        }
    }

    return data.window_should_close;
}

void
quit()
{
    if (data.window != nullptr)
    {
        SDL_DestroyWindow(data.window);
    }

    SDL_Quit();
}

} // namespace goose

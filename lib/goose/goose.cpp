#include "goose/goose.hpp"

#include "goose/common/util.hpp"
#include "goose/render/render.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

struct Data {
    const char *app_name;
    u32 app_version;

    SDL_Window *window;
    bool window_should_close;

    goose::render::RenderContext render_ctx;
};

static Data data;

bool
goose::init(const char *app_name)
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
goose::create_window(const char *title, u32 width, u32 height)
{
    // TODO: Support multiple windows
    if (data.window != nullptr)
    {
        LOG_ERROR("There is already an active window, multiple windows are currently not supported");
        return false;
    }

    bool success;

    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN;

    data.window = SDL_CreateWindow(title, width, height, window_flags);
    if (data.window == nullptr)
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    data.window_should_close = false;

    VkInstance instance;
    if (!goose::render::create_instance(&data.render_ctx, data.app_name, data.app_version, &instance))
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(data.window, instance, nullptr, &surface))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    VkExtent2D window_extent = {
        .width = width,
        .height = height,
    };

    if (!goose::render::init(&data.render_ctx, window_extent, surface))
    {
        LOG_ERROR("Failed to initialize Vulkan renderer");
        return false;
    }

    return true;
}

bool
goose::window_should_close()
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
goose::quit()
{
    goose::render::cleanup(&data.render_ctx);

    if (data.window != nullptr)
    {
        SDL_DestroyWindow(data.window);
    }

    SDL_Quit();
}

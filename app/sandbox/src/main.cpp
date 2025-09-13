#include "goose/core/types.hpp"
#include "goose/core/util.hpp"
#include "goose/graphics/render.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

struct AppData {
    SDL_Window *window;

    goose::RenderData render_data;
};

bool
init(AppData *data)
{
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN;

    data->window = SDL_CreateWindow(
        "Sandbox",
        data->render_data.window_extent.width,
        data->render_data.window_extent.height,
        window_flags);

    if (data->window == nullptr)
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    u32 extension_count;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    for (int i = 0; i < extension_count; ++i)
    {
        data->render_data.instance_extensions.push_back(extensions[i]);
    }

    if (!goose::create_instance(&data->render_data, "Sandbox", VK_MAKE_VERSION(0, 0, 1)))
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }

    if (!SDL_Vulkan_CreateSurface(data->window, data->render_data.instance, nullptr, &data->render_data.surface))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    if (!goose::init_vulkan(&data->render_data))
    {
        LOG_ERROR("Failed to initialize Vulkan");
        return false;
    }

    return true;
}

bool
run(AppData *data)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            return false;
        case SDL_EVENT_WINDOW_RESIZED:
            LOG_DEBUG("Window resized");
            break;
        case SDL_EVENT_WINDOW_OCCLUDED: // NOTE: Interesting for wayland (switching workspaces, etc.)
            LOG_DEBUG("Window occluded");
            break;
        }
    }

    return true;
}

void
cleanup(AppData *data)
{
    goose::cleanup(&data->render_data);

    if (data->window != nullptr)
    {
        SDL_DestroyWindow(data->window);
    }

    SDL_Quit();
}

int
main(int argc, char **argv)
{
    AppData data = {};
    data.render_data.window_extent.width = 1600;
    data.render_data.window_extent.height = 900;

    if (!init(&data))
    {
        cleanup(&data);
        return EXIT_FAILURE;
    }

    while (run(&data))
    {
    }

    cleanup(&data);

    return EXIT_SUCCESS;
}

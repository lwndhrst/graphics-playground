#include "goose/core/types.h"

#include "SDL3/SDL.h"

struct AppData {
    SDL_Window *window;
    VkExtent2D window_extent;
};

bool
init(AppData *data)
{
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        fmt::println("{}", SDL_GetError());
        return false;
    }

    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN;

    data->window = SDL_CreateWindow(
        "Sandbox",
        data->window_extent.width,
        data->window_extent.height,
        window_flags);

    if (data->window == nullptr)
    {
        fmt::println("{}", SDL_GetError());
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
            fmt::println("Window resized");
            break;
        case SDL_EVENT_WINDOW_OCCLUDED: // NOTE: Interesting for wayland (switching workspaces, etc.)
            fmt::println("Window occluded");
            break;
        }
    }

    return true;
}

void
cleanup(AppData *data)
{
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
    data.window_extent.width = 1600;
    data.window_extent.height = 900;

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

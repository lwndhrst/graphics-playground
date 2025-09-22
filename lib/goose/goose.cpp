#include "goose/goose.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"

#include "SDL3/SDL_init.h"

struct Data {
    const char *app_name;
    u32 app_version;
    bool app_is_running;
};

static Data s_data = {};

bool
goose::init(const char *app_name)
{
    // TODO: Make app version configurable
    s_data.app_name = app_name;
    s_data.app_version = VK_MAKE_VERSION(0, 1, 0);

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    if (!goose::render::create_instance(s_data.app_name, s_data.app_version))
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }

    s_data.app_is_running = true;

    return true;
}

void
goose::quit()
{
    goose::render::destroy_device();
    goose::render::destroy_instance();

    SDL_Quit();
}

bool
goose::should_run()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            s_data.app_is_running = false;
            break;

        // Window events
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            get_window_by_id(event.window.windowID)->event_flags.close_requested = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            get_window_by_id(event.window.windowID)->event_flags.resized = true;
            break;
        case SDL_EVENT_WINDOW_OCCLUDED:
            // NOTE: Interesting for wayland (switching workspaces, etc.)
            get_window_by_id(event.window.windowID)->event_flags.occluded = true;
            break;
        }
    }

    return s_data.app_is_running;
}

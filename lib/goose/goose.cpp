#include "goose/goose.hpp"

#include "goose/common/util.hpp"
#include "goose/render/context.hpp"
#include "goose/render/instance.hpp"
#include "goose/window/window.hpp"

#include "SDL3/SDL_init.h"

struct Data {
    const char *app_name;
    u32 app_version;
    bool app_is_running;

    goose::Window window;
    goose::render::RenderContext render_ctx;
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

bool
goose::create_window(const char *title, u32 width, u32 height)
{
    // TODO: Support multiple windows
    if (s_data.window.handle != nullptr)
    {
        LOG_ERROR("There is already an active window, multiple windows are currently not supported");
        return false;
    }

    if (!goose::create_window(title, width, height, s_data.window))
    {
        LOG_ERROR("Failed to create window");
        return false;
    }

    if (!goose::render::create_render_context(s_data.window, s_data.render_ctx))
    {
        LOG_ERROR("Failed to initialize Vulkan renderer");
        return false;
    }

    return true;
}

bool
goose::run()
{
    SDL_Event event;

    // TODO: Support multiple windows
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            s_data.app_is_running = false;
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            s_data.window.should_close = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            fmt::println("Window resized");
            break;
        case SDL_EVENT_WINDOW_OCCLUDED: // NOTE: Interesting for wayland (switching workspaces, etc.)
            fmt::println("Window occluded");
            break;
        }
    }

    return s_data.app_is_running && !s_data.window.should_close;
}

void
goose::quit()
{
    goose::render::destroy_render_context(s_data.render_ctx);
    goose::destroy_window(s_data.window);
    goose::render::destroy_instance();

    SDL_Quit();
}

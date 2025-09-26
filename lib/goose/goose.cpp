#include "goose/goose.hpp"

#include "goose/common/log.hpp"
#include "goose/imgui/imgui.hpp"
#include "goose/render/allocator.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"

#include "SDL3/SDL_init.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"

struct Data {
    const char *app_name;
    u32 app_version;
    bool app_is_running = false;

    bool imgui_is_enabled = false;
};

static Data g_data = {};

bool
goose::init(const char *app_name)
{
    // TODO: Make app version configurable
    g_data.app_name = app_name;
    g_data.app_version = VK_MAKE_VERSION(0, 1, 0);

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        LOG_ERROR("{}", SDL_GetError());
        return false;
    }

    if (!goose::render::create_instance(g_data.app_name, g_data.app_version))
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }

    g_data.app_is_running = true;

    return true;
}

void
goose::quit()
{
    goose::render::destroy_allocator();
    goose::render::destroy_device();
    goose::render::destroy_instance();

    SDL_Quit();
}

void
goose::init_imgui(const WindowInfo &window, const goose::render::RenderContext &ctx)
{

    if (!goose::init_imgui_internal(window, ctx))
    {
        LOG_ERROR("Failed to initialize imgui");
        return;
    }

    g_data.imgui_is_enabled = true;
}

void
goose::quit_imgui()
{
    goose::quit_imgui_internal();

    g_data.imgui_is_enabled = false;
}

bool
goose::should_run()
{
    SDL_Event event;
    WindowInfo *window;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            g_data.app_is_running = false;
            break;

        // Window events
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            window = WindowInfo::get_by_id(event.window.windowID);
            window->event_flags.close_requested = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            window = WindowInfo::get_by_id(event.window.windowID);
            window->event_flags.resized = true;
            window->extent.width = event.window.data1;
            window->extent.height = event.window.data2;
            break;
        case SDL_EVENT_WINDOW_OCCLUDED:
            // NOTE: Interesting for wayland (switching workspaces, etc.)
            window = WindowInfo::get_by_id(event.window.windowID);
            window->event_flags.occluded = true;
            break;
        }

        if (g_data.imgui_is_enabled)
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
    }

    if (g_data.imgui_is_enabled)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
    }

    return g_data.app_is_running;
}

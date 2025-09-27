#pragma once

#include "goose/common/types.hpp"

#include "goose/common/assert.hpp"

struct SDL_Window;
typedef u32 SDL_WindowID;
typedef u64 SDL_WindowFlags;

namespace goose {

struct WindowEventFlags {
    bool close_requested;
    bool resized;
    bool occluded;
};

struct WindowInfo {
    inline static std::vector<goose::WindowInfo *> s_active_windows;

    SDL_Window *window;
    SDL_WindowID id;
    SDL_WindowFlags flags;

    VkExtent2D extent;
    VkSurfaceKHR surface;

    WindowEventFlags event_flags;

    static WindowInfo *get_by_id(SDL_WindowID id, u32 *index = nullptr)
    {
        for (u32 i = 0; i < s_active_windows.size(); ++i)
        {
            WindowInfo *window = s_active_windows[i];
            if (window->id == id)
            {
                if (index != nullptr)
                {
                    *index = i;
                }

                return window;
            }
        }

        ASSERT(false, "Couldn't find window matching the given ID");
        return nullptr;
    }
};

bool create_window(WindowInfo &window, const char *title, u32 width, u32 height, bool resizable = true);
void destroy_window(WindowInfo &window);

}; // namespace goose

#pragma once

#include "goose/common/types.hpp"

struct SDL_Window;
typedef u32 SDL_WindowID;
typedef u64 SDL_WindowFlags;

namespace goose {

struct WindowEventFlags {
    bool close_requested;
    bool resized;
    bool occluded;
};

struct Window {
    SDL_Window *window;
    SDL_WindowID id;
    SDL_WindowFlags flags;

    VkExtent2D extent;
    VkSurfaceKHR surface;

    WindowEventFlags event_flags;
};

bool create_window(const char *title, u32 width, u32 height, Window &window, bool resizable = true);
void destroy_window(Window &window);

Window *get_window_by_id(SDL_WindowID id);
Window *get_window_by_id(SDL_WindowID id, usize &index);

}; // namespace goose

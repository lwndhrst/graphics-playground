#pragma once

#include "goose/common/types.hpp"

struct SDL_Window;
typedef u32 SDL_WindowID;
typedef u64 SDL_WindowFlags;

namespace goose {

struct Window {
    SDL_Window *handle;
    SDL_WindowID id;
    SDL_WindowFlags flags;

    bool should_close;

    VkExtent2D extent;
    VkSurfaceKHR surface;
};

bool create_window(const char *title, u32 width, u32 height, Window &window);
void destroy_window(Window &window);

}; // namespace goose

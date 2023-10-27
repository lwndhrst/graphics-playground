#include "engine.h"
#include "log.h"
#include "renderer.h"

#define WINDOW_TITLE "Graphics Playground"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

namespace gp {

bool
Engine::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        log::error("Failed to initialize SDL2");
        return false;
    }

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (!window) {
        log::error("Failed to create SDL2 window");
        return false;
    }

    Renderer renderer = Renderer();
    if (!renderer.init(window)) {
        log::error("Failed to initialize renderer");
        return false;
    }

    this->window = window;
    this->renderer = renderer;

    return true;
}

void
Engine::run() {
    SDL_Event event;
    for (;;) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                return;

            default:
                break;
            }
        }

        renderer.draw();
    }
}

void
Engine::cleanup() {
    this->renderer.cleanup();
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

} // namespace gp

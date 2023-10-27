#pragma once

#include "renderer.h"

#include <SDL2/SDL.h>

namespace gp {

class Engine {
private:
    SDL_Window *window;
    Renderer renderer;

public:
    bool init();
    void run();
    void cleanup();
};

} // namespace gp

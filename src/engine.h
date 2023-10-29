#pragma once

#include "renderer.h"

#include <SDL2/SDL.h>

namespace gp {

class Engine {
private:
    SDL_Window *window;
    Renderer renderer;
    bool init();
    void cleanup();

public:
    bool run();
};

} // namespace gp

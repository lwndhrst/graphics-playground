#pragma once

#include <SDL2/SDL.h>

namespace gp {

struct RendererData {};

class Renderer {
private:
  RendererData data;

public:
  bool init(SDL_Window *window);
  void draw();
  void cleanup();
};

} // namespace gp

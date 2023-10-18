#include "renderer.h"

#include <vulkan/vulkan.h>

#include <cstdio>

namespace gp {

bool Renderer::init(SDL_Window *window) {
  fprintf(stdout, "Called gp::Renderer::init()\n");
  return true;
}

void Renderer::draw() {
  fprintf(stdout, "Called gp::Renderer::draw()\n");
  return;
}

void Renderer::cleanup() {
  fprintf(stdout, "Called gp::Renderer::cleanup()\n");
  return;
}

} // namespace gp

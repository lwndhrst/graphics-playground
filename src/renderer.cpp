#include "renderer.h"
#include "log.h"

#include <vulkan/vulkan.h>

namespace gp {

bool Renderer::init(SDL_Window *window) {
  gp::log::debug("Called gp::Renderer::init()");
  return true;
}

void Renderer::draw() {
  gp::log::debug("Called gp::Renderer::draw()");
  return;
}

void Renderer::cleanup() {
  gp::log::debug("Called gp::Renderer::cleanup()");
  return;
}

} // namespace gp

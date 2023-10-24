#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

namespace gp {

class Renderer {
private:
  SDL_Window *window;
  VkInstance instance;
  VkSurfaceKHR surface;

  VkResult create_instance();
  SDL_bool create_surface();

public:
  bool init(SDL_Window *window);
  void draw();
  void cleanup();
};

static void print_available_extensions();
static void print_available_layers();

} // namespace gp

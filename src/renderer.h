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
  VkPhysicalDevice physical_device;

  bool create_instance();
  bool create_surface();
  bool create_physical_device();

public:
  bool init(SDL_Window *window);
  void draw();
  void cleanup();
};

} // namespace gp

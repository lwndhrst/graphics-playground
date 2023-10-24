#include "renderer.h"
#include "core.h"
#include "log.h"
#include <vulkan/vulkan_core.h>

namespace gp {

bool Renderer::init(SDL_Window *window) {
  this->window = window;

  if (create_instance() != VK_SUCCESS) {
    log::error("Failed to create Vulkan instance");
    return false;
  }

  return true;
}

void Renderer::draw() { return; }

void Renderer::cleanup() { return; }

VkResult Renderer::create_instance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Graphics Playground";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  u32 extension_count;
  SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);

  const char *extension_names[extension_count];
  SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names);

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = extension_count;
  create_info.ppEnabledExtensionNames = extension_names;
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = nullptr;

  return vkCreateInstance(&create_info, nullptr, &instance);
}

} // namespace gp

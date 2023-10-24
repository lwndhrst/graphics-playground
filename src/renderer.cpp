#include "renderer.h"
#include "core.h"
#include "log.h"
#include <SDL2/SDL_vulkan.h>

namespace gp {

bool Renderer::init(SDL_Window *window) {
  this->window = window;

  print_available_extensions();

  if (create_instance() != VK_SUCCESS) {
    log::error("Failed to create Vulkan instance");
    return false;
  }

  if (create_surface() != SDL_TRUE) {
    log::error("Failed to create Vulkan instance");
    return false;
  }

  return true;
}

void Renderer::draw() {}

void Renderer::cleanup() {
  vkDestroyInstance(instance, nullptr);
}

VkResult Renderer::create_instance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Graphics Playground";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  u32 ext_count;
  SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr);

  const char *ext_names[ext_count];
  SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names);

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = ext_count;
  create_info.ppEnabledExtensionNames = ext_names;
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = nullptr;

  return vkCreateInstance(&create_info, nullptr, &instance);
}

SDL_bool Renderer::create_surface() {
  return SDL_Vulkan_CreateSurface(window, instance, &surface);
}

static void print_available_extensions() {
  u32 ext_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

  VkExtensionProperties ext_props[ext_count];
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, ext_props);

  log::debug("Available Vulkan Extensions:");
  for (auto ext : ext_props)
    log::debug(ext.extensionName);
}

} // namespace gp

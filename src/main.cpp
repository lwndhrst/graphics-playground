#include "log.h"
#include "renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define WINDOW_TITLE "Graphics Playground"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    gp::log::error("Failed to initialize SDL2");
    return EXIT_FAILURE;
  }

  SDL_Window *window = SDL_CreateWindow(
      WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      WINDOW_WIDTH, WINDOW_HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

  if (!window) {
    gp::log::error("Failed to create window");
    return EXIT_FAILURE;
  }

  gp::Renderer renderer = gp::Renderer();
  if (!renderer.init(window)) {
    gp::log::error("Failed to initialize renderer");
    return EXIT_FAILURE;
  }

  SDL_Event event;
  for (;;) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        goto cleanup;

      default:
        break;
      }
    }

    renderer.draw();
  }

cleanup:
  renderer.cleanup();
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}

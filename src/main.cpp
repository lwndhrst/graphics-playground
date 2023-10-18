#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <cstdio>
#include <cstdlib>

#define WINDOW_TITLE "Graphics Playground"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Failed to initialize SDL2\n");
    return EXIT_FAILURE;
  }

  if (SDL_Vulkan_LoadLibrary(nullptr) < 0) {
    fprintf(stderr, "Failed to load Vulkan\n");
    return EXIT_FAILURE;
  }

  SDL_Window *window = SDL_CreateWindow(
      WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      WINDOW_WIDTH, WINDOW_HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    return EXIT_FAILURE;
  }

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  if (!surface) {
    fprintf(stderr, "Failed to get window surface\n");
    return EXIT_FAILURE;
  }

  SDL_Event event;
  for (;;) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        goto cleanup;

      default:
        SDL_UpdateWindowSurface(window);
        break;
      }
    }
  }

cleanup:
  SDL_DestroyWindowSurface(window);
  SDL_DestroyWindow(window);
  SDL_Vulkan_UnloadLibrary();
  SDL_Quit();

  return EXIT_SUCCESS;
}

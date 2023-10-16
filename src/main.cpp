#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
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

  SDL_Window *window = SDL_CreateWindow(
      WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    return EXIT_FAILURE;
  }

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  if (!surface) {
    fprintf(stderr, "Failed to get window surface\n");
    return EXIT_FAILURE;
  }

  bool running = true;
  while (running) {
    SDL_Event windowEvent;
    while (SDL_PollEvent(&windowEvent)) {
      switch (windowEvent.type) {
      case SDL_QUIT:
        running = false;
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
  SDL_Quit();

  return EXIT_SUCCESS;
}

#include "window.h"

#include <cstdio>
#include <cstdlib>

int main() {
  auto maybe_window_data = gp::x_window::open();
  if (!maybe_window_data.has_value()) {
    fprintf(stderr, "Couldn't open window\n");
    exit(EXIT_FAILURE);
  }

  auto window_data = maybe_window_data.value();
  gp::x_window::event_loop(window_data);

  return EXIT_SUCCESS;
}

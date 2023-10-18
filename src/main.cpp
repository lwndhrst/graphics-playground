#include "log.h"
#include "engine.h"

int main(int argc, char **argv) {

  gp::Engine engine = gp::Engine();
  if (!engine.init()) {
    gp::log::error("Failed to start engine");
    return EXIT_FAILURE;
  }

  // main loop
  engine.run();

  engine.cleanup();

  return EXIT_SUCCESS;
}

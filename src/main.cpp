#include "engine.h"
#include "log.h"

int
main(int argc, char **argv)
{
    using namespace gp;

    Engine engine = Engine();
    if (!engine.init()) {
        log::error("Failed to start engine");
        return EXIT_FAILURE;
    }

    // main loop
    engine.run();

    engine.cleanup();

    return EXIT_SUCCESS;
}

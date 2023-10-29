#include "engine.h"
#include "log.h"

int
main(int argc, char **argv)
{
    using namespace gp;

    Engine engine;
    if (!engine.run()) {
        log::error("Failed to start engine");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

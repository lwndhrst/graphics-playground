#include "goose/goose.hpp"

int
main(int argc, char **argv)
{
    goose::init("Sandbox");

    if (!goose::create_window("Sandbox", 800, 600))
    {
        goose::quit();

        return EXIT_FAILURE;
    }

    while (goose::should_run())
    {
        // Main loop
        goose::render::begin_frame();
        goose::render::end_frame();
    }

    goose::quit();

    return EXIT_SUCCESS;
}

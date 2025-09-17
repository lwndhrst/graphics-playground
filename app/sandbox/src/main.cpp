#include "goose/goose.hpp"

int
main(int argc, char **argv)
{
    goose::init("Sandbox");

    if (!goose::create_window("Sandbox", 1600, 900))
    {
        goose::quit();

        return EXIT_FAILURE;
    }

    while (!goose::window_should_close())
    {
        // Main loop
    }

    goose::quit();

    return EXIT_SUCCESS;
}

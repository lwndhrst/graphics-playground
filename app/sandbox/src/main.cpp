#include "goose/goose.hpp"

int
main(int argc, char **argv)
{
    goose::init("Sandbox");

    goose::create_window("Sandbox", 1600, 900);

    while (!goose::window_should_close())
    {
        // Main loop
    }

    goose::quit();

    return 0;
}

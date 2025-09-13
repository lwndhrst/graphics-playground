#include "goose/goose.hpp"

#include <vulkan/vulkan_core.h>

int
main(int argc, char **argv)
{
    goose::init("Sandbox", VK_MAKE_VERSION(0, 0, 1));

    goose::create_window("Sandbox", 1600, 900);

    while (!goose::window_should_close())
    {
        goose::update();
    }

    goose::quit();

    return 0;
}

#include "goose/goose.hpp"

#define APP_NAME "Sandbox"

static goose::Window window;
static goose::render::RenderContext render_context;

bool
init()
{
    goose::init(APP_NAME);

    if (!goose::create_window(APP_NAME, 800, 600, window))
    {
        LOG_ERROR("Failed to create window");
        return false;
    }

    if (!goose::render::create_render_context(window, render_context))
    {
        LOG_ERROR("Failed to initialize vulkan renderer");
        return false;
    }

    return true;
}

void
run()
{
    while (goose::should_run())
    {
        auto [cmd, img] = goose::render::begin_frame(render_context);

        VkClearColorValue clear_color_value = {{0.0f, 0.0f, 1.0f, 1.0f}};
        VkImageSubresourceRange clear_subresource_range = goose::render::make_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmd, img, VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1, &clear_subresource_range);

        goose::render::end_frame(render_context);
    }
}

void
cleanup()
{
    goose::render::destroy_render_context(render_context);
    goose::destroy_window(window);
    goose::quit();
}

int
main(int argc, char **argv)
{
    if (!init())
    {
        return EXIT_FAILURE;
    }

    run();

    cleanup();

    return EXIT_SUCCESS;
}

#include "goose/goose.hpp"

#define APP_NAME "Sandbox"

static goose::Window window;
static goose::render::RenderContext render_context;

// Use an extra image as draw target rather than directly drawing into swapchain images
static goose::render::Image draw_image;
static VkExtent2D draw_extent;

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
        LOG_ERROR("Failed to create render context");
        return false;
    }

    goose::render::ImageBuilder image_builder = goose::render::ImageBuilder();
    image_builder
        .set_extent(window.extent)
        .set_format(VK_FORMAT_R16G16B16A16_SFLOAT)
        .add_usage_flags(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .add_usage_flags(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .add_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(goose::render::MEMORY_USAGE_GPU_ONLY);

    if (!image_builder.build(draw_image))
    {
        LOG_ERROR("Failed to create draw image");
        return false;
    }

    draw_extent = window.extent;

    goose::render::add_cleanup_callback(render_context, [&]() {
        goose::render::destroy_image(draw_image);
    });

    return true;
}

void
run()
{
    while (goose::should_run())
    {
        // Start recording commands for the current frame
        auto [cmd, swapchain_image] = goose::render::begin_frame(render_context);

        // Transition draw image to a usable
        goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkClearColorValue clear_color_value = {{0.0f, 0.0f, 1.0f, 1.0f}};
        VkImageSubresourceRange clear_subresource_range = goose::render::make_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1, &clear_subresource_range);

        // Copy draw image content to swapchain image
        goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        goose::render::copy_image_to_image(cmd, draw_image.image, swapchain_image.image, draw_extent, swapchain_image.extent);
        goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // Finish recording commands for the current frame
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

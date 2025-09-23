#include "goose/goose.hpp"

#define APP_NAME "Sandbox"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

static goose::Window window;
static goose::render::RenderContext render_context;

// Use an extra image as draw target rather than directly drawing into swapchain images
static goose::render::Image draw_image;
static VkExtent2D draw_image_extent;

static VkDescriptorPool descriptor_pool;
static VkDescriptorSet descriptor_set;
static VkDescriptorSetLayout descriptor_set_layout;

bool
init_draw_image(VkExtent2D extent)
{
    draw_image_extent = extent;

    goose::render::ImageBuilder image_builder;
    image_builder
        .set_extent(extent)
        .set_format(VK_FORMAT_R16G16B16A16_SFLOAT)
        .add_usage_flags(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .add_usage_flags(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .add_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(goose::render::MEMORY_USAGE_GPU_ONLY);

    if (!image_builder.build(draw_image))
    {
        return false;
    }

    goose::render::add_cleanup_callback(render_context, [&]() {
        goose::render::destroy_image(draw_image);
    });

    return true;
}

bool
init_descriptors()
{
    // TODO: Make this nices to use

    u32 max_descriptor_sets = 10;
    std::vector<VkDescriptorPoolSize> descriptor_count_per_type = {
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = max_descriptor_sets,
        },
    };

    descriptor_pool = goose::render::create_descriptor_pool(max_descriptor_sets, descriptor_count_per_type);

    goose::render::DescriptorSetLayoutBuilder descriptor_set_layout_builder;
    descriptor_set_layout_builder
        .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    if (!descriptor_set_layout_builder.build(descriptor_set_layout, VK_SHADER_STAGE_COMPUTE_BIT))
    {
        return false;
    }

    descriptor_set = goose::render::allocate_descriptor_set(descriptor_pool, descriptor_set_layout);

    VkDescriptorImageInfo descriptor_image_info = {
        .imageView = draw_image.view,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };

    VkWriteDescriptorSet write_descriptor_set = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &descriptor_image_info,
    };

    vkUpdateDescriptorSets(goose::render::get_device().logical, 1, &write_descriptor_set, 0, nullptr);

    goose::render::add_cleanup_callback(render_context, [&]() {
        goose::render::destroy_descriptor_set_layout(descriptor_set_layout);
        goose::render::destroy_descriptor_pool(descriptor_pool);
    });

    return true;
}

bool
init()
{
    goose::init(APP_NAME);

    if (!goose::create_window(window, APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        LOG_ERROR("Failed to create window");
        return false;
    }

    if (!goose::render::create_render_context(render_context, window))
    {
        LOG_ERROR("Failed to create render context");
        return false;
    }

    if (!init_draw_image(window.extent))
    {
        LOG_ERROR("Failed to create draw image");
        return false;
    }

    if (!init_descriptors())
    {
        LOG_ERROR("Failed to create descriptors");
        return false;
    }

    return true;
}

void
draw()
{
    // Start recording commands for the current frame
    auto [cmd, swapchain_image] = goose::render::begin_frame(render_context);

    // Transition draw image to a usable layout
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clear_color_value = {{0.0f, 0.0f, 1.0f, 1.0f}};
    VkImageSubresourceRange clear_subresource_range = goose::render::make_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1, &clear_subresource_range);

    // Copy draw image content to swapchain image
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    goose::render::copy_image_to_image(cmd, draw_image.image, swapchain_image.image, draw_image_extent, swapchain_image.extent);
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Finish recording commands for the current frame
    goose::render::end_frame(render_context);
}

void
run()
{
    while (goose::should_run())
    {
        if (window.event_flags.resized)
        {
            goose::render::resize_swapchain(render_context, window);
            window.event_flags.resized = false;
        }

        draw();
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

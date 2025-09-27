#include "goose/goose.hpp"

#include "goose/imgui/imgui.hpp"

#define APP_NAME "Sandbox"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

static goose::WindowInfo window;
static goose::render::RenderContext render_context;

// Use an extra image as draw target rather than directly drawing into swapchain images
static goose::render::ImageInfo draw_image;
static VkExtent2D draw_image_extent;

static VkDescriptorPool descriptor_pool;
static VkDescriptorSet descriptor_set;
static VkDescriptorSetLayout descriptor_set_layout;

static VkPipeline pipeline;
static VkPipelineLayout pipeline_layout;

bool
init_draw_image(VkExtent2D extent)
{
    draw_image_extent = extent;

    goose::render::ImageBuilder image_builder(goose::render::IMAGE_TYPE_2D);

    image_builder
        .set_extent({extent.width, extent.height, 1})
        .set_format(VK_FORMAT_R16G16B16A16_SFLOAT)
        .set_usage_flags(
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
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
    // TODO: Make this nicer to use

    const VkDevice &device = goose::render::Device::get();

    u32 max_descriptor_sets = 1;
    std::vector<VkDescriptorPoolSize> max_descriptor_count_per_type = {
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
        },
    };

    descriptor_pool = goose::render::create_descriptor_pool(max_descriptor_sets, max_descriptor_count_per_type);

    goose::render::DescriptorSetLayoutBuilder descriptor_set_layout_builder;
    descriptor_set_layout_builder
        .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        .set_stage_flags(VK_SHADER_STAGE_COMPUTE_BIT);

    if (!descriptor_set_layout_builder.build(descriptor_set_layout))
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

    vkUpdateDescriptorSets(device, 1, &write_descriptor_set, 0, nullptr);

    goose::render::add_cleanup_callback(render_context, [&]() {
        goose::render::destroy_descriptor_set_layout(descriptor_set_layout);
        goose::render::destroy_descriptor_pool(descriptor_pool);
    });

    return true;
}

bool
init_pipeline()
{
    // TODO: Make this nicer to use

    const VkDevice &device = goose::render::Device::get();

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptor_set_layout,
    };

    vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);

    VkShaderModule shader_module = goose::render::create_shader_module(SHADER_PATH "/gradient.comp.spv");

    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = shader_module,
        .pName = "main",
    };

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = pipeline_shader_stage_create_info,
        .layout = pipeline_layout,
    };

    // TODO: Pipeline cache?
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &pipeline);

    // Shader module can be destroyed directly
    goose::render::destroy_shader_module(shader_module);

    goose::render::add_cleanup_callback(render_context, [&]() {
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);
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

    if (!init_draw_image({1920, 1080}))
    {
        LOG_ERROR("Failed to create draw image");
        return false;
    }

    if (!init_descriptors())
    {
        LOG_ERROR("Failed to create descriptors");
        return false;
    }

    if (!init_pipeline())
    {
        LOG_ERROR("Failed to create compute pipeline");
        return false;
    }

    goose::init_imgui(window, render_context);

    return true;
}

void
draw()
{
    // Start recording commands for the current frame
    auto [cmd, swapchain_image] = goose::render::begin_frame(render_context);

    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &command_buffer_begin_info);

    // Execute compute pipeline dispatch with 16x16 workgroup size
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    vkCmdDispatch(cmd, UINT_DIV_CEIL(draw_image_extent.width, 16), UINT_DIV_CEIL(draw_image_extent.height, 16), 1);

    // Copy draw image content to swapchain image
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    goose::render::copy_image_to_image(cmd, draw_image.image, swapchain_image.image, draw_image_extent, swapchain_image.extent);

    // Draw ImGui directly into swapchain image
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    goose::render::draw_imgui(cmd, swapchain_image.view, swapchain_image.extent);

    // Swapchain image should in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR by the end of the command buffer
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(cmd);

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

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        draw();
    }
}

void
cleanup()
{
    goose::quit_imgui();

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

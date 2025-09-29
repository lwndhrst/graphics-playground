#include "goose/goose.hpp"

#include "goose/imgui/imgui.hpp"

#define APP_NAME "Sandbox"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_FRAMES_IN_FLIGHT 2

static goose::WindowInfo window;
static goose::render::RenderContext render_context;

// Use an extra image as draw target rather than directly drawing into swapchain images
static goose::render::ImageInfo draw_images[MAX_FRAMES_IN_FLIGHT];

// Gradient parameters for the shader
struct PushConstants {
    glm::vec4 color_a;
    glm::vec4 color_b;
};

static PushConstants push_constants = {
    .color_a = {1.0f, 0.25f, 0.0f, 1.0f},
    .color_b = {0.0f, 0.25f, 1.0f, 1.0f},
};

static VkDescriptorPool descriptor_pool;
static VkDescriptorSetLayout descriptor_set_layout;
static VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];

static VkPipeline pipeline;
static VkPipelineLayout pipeline_layout;

bool
init_draw_images(VkExtent2D extent)
{
    goose::render::ImageBuilder image_builder(goose::render::IMAGE_TYPE_2D);
    image_builder
        .set_extent(extent)
        .set_format(VK_FORMAT_R16G16B16A16_SFLOAT)
        .set_usage_flags(
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(goose::render::MEMORY_USAGE_GPU_ONLY);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (!image_builder.build(draw_images[i]))
        {
            return false;
        }

        goose::render::add_cleanup_callback(render_context, [i]() {
            goose::render::destroy_image(draw_images[i]);
        });
    }

    return true;
}

bool
init_descriptors()
{
    u32 max_descriptor_sets = MAX_FRAMES_IN_FLIGHT;
    std::vector<VkDescriptorPoolSize> max_descriptor_count_per_type = {
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT,
        },
    };

    descriptor_pool = goose::render::create_descriptor_pool(max_descriptor_sets, max_descriptor_count_per_type);

    goose::render::DescriptorSetLayoutBuilder descriptor_set_layout_builder = {};
    descriptor_set_layout_builder
        .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    if (!descriptor_set_layout_builder.build(descriptor_set_layout, VK_SHADER_STAGE_COMPUTE_BIT))
    {
        return false;
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        // TODO: Make writing descriptors nicer somehow

        descriptor_sets[i] = goose::render::allocate_descriptor_set(descriptor_pool, descriptor_set_layout);

        VkDescriptorImageInfo descriptor_image_info = {
            .imageView = draw_images[i].view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        VkWriteDescriptorSet write_descriptor_set = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptor_sets[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &descriptor_image_info,
        };

        vkUpdateDescriptorSets(goose::render::Device::get(), 1, &write_descriptor_set, 0, nullptr);
    }

    goose::render::add_cleanup_callback(render_context, []() {
        goose::render::destroy_descriptor_set_layout(descriptor_set_layout);
        goose::render::destroy_descriptor_pool(descriptor_pool);
    });

    return true;
}

bool
init_pipeline()
{
    goose::render::PipelineLayoutBuilder pipeline_layout_builder = {};
    pipeline_layout_builder
        .add_descriptor_set_layout(descriptor_set_layout)
        .add_push_constant(0, sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);

    if (!pipeline_layout_builder.build(pipeline_layout))
    {
        LOG_ERROR("Failed to create pipeline layout");
        return false;
    }

    goose::render::PipelineBuilder pipeline_builder(goose::render::PIPELINE_TYPE_COMPUTE);
    pipeline_builder
        .add_shader(SHADER_PATH "/gradient.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);

    if (!pipeline_builder.build(pipeline, pipeline_layout))
    {
        LOG_ERROR("Failed to create pipeline");
        return false;
    }

    goose::render::add_cleanup_callback(render_context, [&]() {
        goose::render::destroy_pipeline(pipeline);
        goose::render::destroy_pipeline_layout(pipeline_layout);
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

    goose::render::FrameDataCreateInfo frame_data_create_info = {
        .max_frames_in_flight = MAX_FRAMES_IN_FLIGHT,
    };

    if (!goose::render::create_render_context(render_context, window, frame_data_create_info))
    {
        LOG_ERROR("Failed to create render context");
        return false;
    }

    if (!init_draw_images({1920, 1080}))
    {
        LOG_ERROR("Failed to create draw images");
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

    goose::enable_imgui(window, render_context);

    return true;
}

void
draw()
{
    auto [frame, swapchain_image] = goose::render::begin_frame(render_context);

    const goose::render::ImageInfo &draw_image = draw_images[frame.index];

    // Start recording commands for the current frame
    VkCommandBuffer cmd = frame.main_command_buffer;
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &command_buffer_begin_info);

    // Execute compute pipeline dispatch with 16x16 workgroup size
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_sets[frame.index], 0, nullptr);
    vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push_constants);
    vkCmdDispatch(cmd, UINT_DIV_CEIL(draw_image.extent.width, 16), UINT_DIV_CEIL(draw_image.extent.height, 16), 1);

    // Copy draw image content to swapchain image
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    goose::render::copy_image_to_image(cmd, draw_image.image, swapchain_image.image, draw_image.extent_2d, swapchain_image.extent);

    // Draw ImGui directly into swapchain image
    goose::render::transition_image(cmd, swapchain_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    goose::imgui::draw_imgui(cmd, swapchain_image.view, swapchain_image.extent);

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

        if (ImGui::Begin("Gradient Parameters"))
        {
            ImGui::ColorEdit4("Color A", reinterpret_cast<float *>(&push_constants.color_a));
            ImGui::ColorEdit4("Color B", reinterpret_cast<float *>(&push_constants.color_b));
            ImGui::End();
        }

        ImGui::Render();

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

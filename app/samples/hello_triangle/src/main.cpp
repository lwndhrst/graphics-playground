#include "goose/goose.hpp"

#include "goose/imgui/imgui.hpp"

#define APP_NAME "Sandbox"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_FRAMES_IN_FLIGHT 2

static goose::WindowInfo window;
static goose::render::RenderContext render_context;

// Use an extra image as draw target rather than directly drawing into swapchain images
VkFormat draw_image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
static goose::render::ImageInfo draw_images[MAX_FRAMES_IN_FLIGHT];

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
        .set_format(draw_image_format)
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

        render_context.cleanup_queue.push({
            .type = goose::render::CLEANUP_QUEUE_ITEM_TYPE_DESTROY_IMAGE,
            .image = &draw_images[i],
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

    render_context.cleanup_queue.push({
        .type = goose::render::CLEANUP_QUEUE_ITEM_TYPE_DESTROY_DESCRIPTOR_POOL,
        .descriptor_pool = descriptor_pool,
    });

    goose::render::DescriptorSetLayoutBuilder descriptor_set_layout_builder = {};
    descriptor_set_layout_builder
        .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    if (!descriptor_set_layout_builder.build(descriptor_set_layout, VK_SHADER_STAGE_COMPUTE_BIT))
    {
        return false;
    }

    render_context.cleanup_queue.push({
        .type = goose::render::CLEANUP_QUEUE_ITEM_TYPE_DESTROY_DESCRIPTOR_SET_LAYOUT,
        .descriptor_set_layout = descriptor_set_layout,
    });

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

    return true;
}

bool
init_pipeline()
{
    goose::render::PipelineLayoutBuilder pipeline_layout_builder = {};
    pipeline_layout_builder
        .add_descriptor_set_layout(descriptor_set_layout);

    if (!pipeline_layout_builder.build(pipeline_layout))
    {
        LOG_ERROR("Failed to create pipeline layout");
        return false;
    }

    render_context.cleanup_queue.push({
        .type = goose::render::CLEANUP_QUEUE_ITEM_TYPE_DESTROY_PIPELINE_LAYOUT,
        .pipeline_layout = pipeline_layout,
    });

    goose::render::PipelineBuilder pipeline_builder(goose::render::PIPELINE_TYPE_GRAPHICS);
    pipeline_builder
        .add_shader(SHADER_PATH "/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader(SHADER_PATH "/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_attachment_format(draw_image_format);

    if (!pipeline_builder.build(pipeline, pipeline_layout))
    {
        LOG_ERROR("Failed to create pipeline");
        return false;
    }

    render_context.cleanup_queue.push({
        .type = goose::render::CLEANUP_QUEUE_ITEM_TYPE_DESTROY_PIPELINE,
        .pipeline = pipeline,
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

    VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = draw_image.view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .color = {{0.0f, 0.0f, 0.0f, 1.0f}},
        },
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .offset = {0, 0},
            .extent = draw_image.extent_2d,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<f32>(draw_image.extent.width),
        .height = static_cast<f32>(draw_image.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = draw_image.extent_2d,
    };

    // Begin render pass
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkCmdBeginRendering(cmd, &rendering_info);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdEndRendering(cmd);

    // Copy draw image content to swapchain image
    goose::render::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
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
        ImGui::ShowDemoWindow();
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

#include "VkBootstrap.h"
#include "volk.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "fmt/core.h"

#include <fstream>

#define APP_NAME "Sandbox"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_FRAMES_IN_FLIGHT 2

static SDL_Window *g_window = nullptr;
static VkSurfaceKHR g_surface = VK_NULL_HANDLE;

static vkb::Instance g_instance;
static vkb::Device g_device;
static vkb::Swapchain g_swapchain;

struct RenderData {
    VkQueue graphics_queue;
    uint32_t graphics_queue_index;

    VkQueue present_queue;
    uint32_t present_queue_index;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkFence> in_flight_fences;
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
};

static RenderData g_render_data;

bool
init_vulkan()
{
    if (VK_SUCCESS != volkInitialize())
    {
        fmt::println("Failed to initialize volk");
        return false;
    }

    vkb::InstanceBuilder instance_builder;
    instance_builder
        .set_app_name(APP_NAME)
        .set_minimum_instance_version(1, 3)
        .request_validation_layers()
        .use_default_debug_messenger();

    auto instance_ret = instance_builder.build();
    if (!instance_ret.has_value())
    {
        fmt::println("Failed to create instance: {}", instance_ret.error().message());
        return false;
    }

    g_instance = instance_ret.value();

    volkLoadInstanceOnly(g_instance);

    if (!SDL_Vulkan_CreateSurface(g_window, g_instance, nullptr, &g_surface))
    {
        fmt::println("Failed to create surface: {}", SDL_GetError());
        return false;
    }

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = {};
    mesh_shader_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    mesh_shader_features.taskShader = VK_TRUE;
    mesh_shader_features.meshShader = VK_TRUE;

    vkb::PhysicalDeviceSelector physical_device_selector(g_instance);
    physical_device_selector
        .set_minimum_version(1, 3)
        .set_surface(g_surface)
        .add_required_extension(VK_EXT_MESH_SHADER_EXTENSION_NAME)
        .add_required_extension_features(mesh_shader_features)
        .set_required_features_13({
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        });

    auto physical_device_ret = physical_device_selector.select();
    if (!physical_device_ret.has_value())
    {
        fmt::println("Failed to select physical device: {}", physical_device_ret.error().message());
        return false;
    }

    fmt::println("Selected physical device: {}", physical_device_ret->name);

    vkb::DeviceBuilder device_builder(physical_device_ret.value());

    auto device_ret = device_builder.build();
    if (!device_ret.has_value())
    {
        fmt::println("Failed to create device: {}", device_ret.error().message());
        return false;
    }

    g_device = device_ret.value();

    volkLoadDevice(g_device);

    return true;
}

bool
get_queues()
{
    auto graphics_queue_ret = g_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret.has_value())
    {
        fmt::println("Failed to get graphics queue: {}", graphics_queue_ret.error().message());
        return false;
    }

    g_render_data.graphics_queue = graphics_queue_ret.value();
    g_render_data.graphics_queue_index = g_device.get_queue_index(vkb::QueueType::graphics).value();

    auto present_queue_ret = g_device.get_queue(vkb::QueueType::present);
    if (!present_queue_ret.has_value())
    {
        fmt::println("Failed to get present queue: {}", present_queue_ret.error().message());
        return false;
    }

    g_render_data.present_queue = present_queue_ret.value();
    g_render_data.present_queue_index = g_device.get_queue_index(vkb::QueueType::present).value();

    return true;
}

int
create_swapchain()
{
    int width, height;
    SDL_GetWindowSize(g_window, &width, &height);

    vkb::SwapchainBuilder swapchain_builder(g_device);
    swapchain_builder
        .set_desired_extent(width, height)
        .set_old_swapchain(g_swapchain);

    auto swapchain_ret = swapchain_builder.build();
    if (!swapchain_ret.has_value())
    {
        fmt::println("Failed to create swapchain: {}", swapchain_ret.error().message());
        return false;
    }

    vkb::destroy_swapchain(g_swapchain);

    g_swapchain = swapchain_ret.value();

    return true;
}

int
get_swapchain_images_and_views()
{
    auto swapchain_images_ret = g_swapchain.get_images();
    if (!swapchain_images_ret.has_value())
    {
        fmt::println("Failed to get swapchain images: {}", swapchain_images_ret.error().message());
        return false;
    }

    auto swapchain_image_views_ret = g_swapchain.get_image_views();
    if (!swapchain_image_views_ret.has_value())
    {
        fmt::println("Failed to get swapchain image views: {}", swapchain_image_views_ret.error().message());
        return false;
    }

    g_render_data.swapchain_images = swapchain_images_ret.value();
    g_render_data.swapchain_image_views = swapchain_image_views_ret.value();

    return true;
}

std::vector<char>
read_file(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(file_size));

    file.close();

    return buffer;
}

VkShaderModule
create_shader_module(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo shader_module_info = {};
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = code.size();
    shader_module_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shader_module;
    if (VK_SUCCESS != vkCreateShaderModule(g_device, &shader_module_info, nullptr, &shader_module))
    {
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

bool
create_pipeline()
{
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;

    if (VK_SUCCESS != vkCreatePipelineLayout(g_device, &pipeline_layout_info, nullptr, &g_render_data.pipeline_layout))
    {
        fmt::println("Failed to create pipeline layout");
        return false;
    }

    VkShaderModule shader_module = create_shader_module(read_file(SHADER_PATH "/triangle.spv"));
    if (VK_NULL_HANDLE == shader_module)
    {
        fmt::println("Failed to create shader module");
        return false;
    }

    VkPipelineShaderStageCreateInfo task_stage = {};
    task_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    task_stage.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    task_stage.module = shader_module;
    task_stage.pName = "taskMain";

    VkPipelineShaderStageCreateInfo mesh_stage = {};
    mesh_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mesh_stage.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    mesh_stage.module = shader_module;
    mesh_stage.pName = "meshMain";

    VkPipelineShaderStageCreateInfo fragment_stage = {};
    fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage.module = shader_module;
    fragment_stage.pName = "fragMain";

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        task_stage,
        mesh_stage,
        fragment_stage,
    };

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = dynamic_states.size();
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineRenderingCreateInfo rendering_info = {};
    rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &g_swapchain.image_format;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = &rendering_info;
    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = g_render_data.pipeline_layout;

    if (VK_SUCCESS != vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &g_render_data.pipeline))
    {
        vkDestroyShaderModule(g_device, shader_module, nullptr);

        fmt::println("Failed to create pipeline");
        return false;
    }

    vkDestroyShaderModule(g_device, shader_module, nullptr);

    return true;
}

bool
create_command_objects()
{
    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = g_render_data.graphics_queue_index;

    if (VK_SUCCESS != vkCreateCommandPool(g_device, &command_pool_info, nullptr, &g_render_data.command_pool))
    {
        fmt::println("Failed to create command pool");
        return false;
    }

    VkCommandBufferAllocateInfo command_buffer_info = {};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.commandPool = g_render_data.command_pool;
    command_buffer_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    g_render_data.command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    if (VK_SUCCESS != vkAllocateCommandBuffers(g_device, &command_buffer_info, g_render_data.command_buffers.data()))
    {
        fmt::println("Failed to create command buffers");
        return false;
    }

    return true;
}

bool
create_synchronization_objects()
{
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    g_render_data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    g_render_data.image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (VK_SUCCESS != vkCreateFence(g_device, &fence_info, nullptr, &g_render_data.in_flight_fences[i]))
        {
            fmt::println("Failed to create in-flight fence");
            return false;
        }

        if (VK_SUCCESS != vkCreateSemaphore(g_device, &semaphore_info, nullptr, &g_render_data.image_available_semaphores[i]))
        {
            fmt::println("Failed to create image-available semaphore");
            return false;
        }
    }

    g_render_data.render_finished_semaphores.resize(g_swapchain.image_count);

    for (size_t i = 0; i < g_swapchain.image_count; ++i)
    {
        if (VK_SUCCESS != vkCreateSemaphore(g_device, &semaphore_info, nullptr, &g_render_data.render_finished_semaphores[i]))
        {
            fmt::println("Failed to create render-finished semaphore");
            return false;
        }
    }

    return true;
}

bool
resize_swapchain()
{
    vkDeviceWaitIdle(g_device);

    g_swapchain.destroy_image_views(g_render_data.swapchain_image_views);

    if (!create_swapchain())
        return false;

    if (!get_swapchain_images_and_views())
        return false;

    return true;
}

void
transition_image_layout(VkCommandBuffer cmd, VkImage img, VkImageLayout from, VkImageLayout to)
{
    VkImageAspectFlags aspect_flags =
        to == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
            ? VK_IMAGE_ASPECT_DEPTH_BIT
            : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = aspect_flags;
    subresource_range.baseMipLevel = 0;
    subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
    subresource_range.baseArrayLayer = 0;
    subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkImageMemoryBarrier2 memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    memory_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    memory_barrier.oldLayout = from;
    memory_barrier.newLayout = to;
    memory_barrier.image = img;
    memory_barrier.subresourceRange = subresource_range;

    VkDependencyInfo dependency_info = {};
    dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &memory_barrier;

    vkCmdPipelineBarrier2(cmd, &dependency_info);
}

void
draw()
{
    // TODO: Error handling

    static uint32_t current_frame = 0;

    vkWaitForFences(g_device, 1, &g_render_data.in_flight_fences[current_frame], true, 1000000000);
    vkResetFences(g_device, 1, &g_render_data.in_flight_fences[current_frame]);

    uint32_t current_swapchain_image;
    vkAcquireNextImageKHR(
        g_device,
        g_swapchain,
        UINT64_MAX,
        g_render_data.image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &current_swapchain_image);

    VkCommandBuffer cmd = g_render_data.command_buffers[current_frame];
    VkImage img = g_render_data.swapchain_images[current_swapchain_image];
    VkImageView view = g_render_data.swapchain_image_views[current_swapchain_image];

    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &command_buffer_begin_info);

    VkRenderingAttachmentInfo color_attachment = {};
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingInfo rendering_info = {};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea = {{0, 0}, g_swapchain.extent};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(g_swapchain.extent.width);
    viewport.height = static_cast<float>(g_swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = g_swapchain.extent;

    transition_image_layout(cmd, img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    vkCmdBeginRendering(cmd, &rendering_info);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_render_data.pipeline);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    uint32_t num_workgroups_x = 1;
    uint32_t num_workgroups_y = 1;
    uint32_t num_workgroups_z = 1;
    vkCmdDrawMeshTasksEXT(cmd, num_workgroups_x, num_workgroups_y, num_workgroups_z);
    vkCmdEndRendering(cmd);

    transition_image_layout(cmd, img, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo command_buffer_submit_info = {};
    command_buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    command_buffer_submit_info.commandBuffer = g_render_data.command_buffers[current_frame];

    VkSemaphoreSubmitInfo wait_semaphore_info = {};
    wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait_semaphore_info.semaphore = g_render_data.image_available_semaphores[current_frame];
    wait_semaphore_info.value = 1;
    wait_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

    VkSemaphoreSubmitInfo signal_semaphore_info = {};
    signal_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signal_semaphore_info.semaphore = g_render_data.render_finished_semaphores[current_swapchain_image];
    signal_semaphore_info.value = 1;
    signal_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

    VkSubmitInfo2 submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_semaphore_info;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_buffer_submit_info;
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;

    vkQueueSubmit2(g_render_data.graphics_queue, 1, &submit_info, g_render_data.in_flight_fences[current_frame]);

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &g_render_data.render_finished_semaphores[current_swapchain_image];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &g_swapchain.swapchain;
    present_info.pImageIndices = &current_swapchain_image;

    vkQueuePresentKHR(g_render_data.present_queue, &present_info);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
run()
{
    SDL_Event event;

    bool window_resized = false;

    for (;;)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                return;
            case SDL_EVENT_WINDOW_RESIZED:
                window_resized = true;
                break;
            }
        }

        if (window_resized)
        {
            if (!resize_swapchain())
                return;

            window_resized = false;
        }

        draw();
    }
}

bool
init()
{
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

    g_window = SDL_CreateWindow(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if (!init_vulkan())
        return false;

    if (!get_queues())
        return false;

    if (!create_swapchain())
        return false;

    if (!get_swapchain_images_and_views())
        return false;

    if (!create_pipeline())
        return false;

    if (!create_command_objects())
        return false;

    if (!create_synchronization_objects())
        return false;

    return true;
}

void
cleanup()
{
    if (VK_NULL_HANDLE != g_device.device)
    {
        vkDeviceWaitIdle(g_device);

        for (size_t i = 0; i < g_swapchain.image_count; ++i)
        {
            vkDestroySemaphore(g_device, g_render_data.render_finished_semaphores[i], nullptr);
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(g_device, g_render_data.image_available_semaphores[i], nullptr);
            vkDestroyFence(g_device, g_render_data.in_flight_fences[i], nullptr);
        }

        vkDestroyCommandPool(g_device, g_render_data.command_pool, nullptr);

        vkDestroyPipeline(g_device, g_render_data.pipeline, nullptr);
        vkDestroyPipelineLayout(g_device, g_render_data.pipeline_layout, nullptr);

        g_swapchain.destroy_image_views(g_render_data.swapchain_image_views);
        vkb::destroy_swapchain(g_swapchain);

        vkb::destroy_device(g_device);
    }

    if (VK_NULL_HANDLE != g_instance.instance)
    {
        SDL_Vulkan_DestroySurface(g_instance, g_surface, nullptr);
        vkb::destroy_instance(g_instance);
    }

    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

int
main(int argc, char **argv)
{
    if (!init())
    {
        cleanup();
        return EXIT_FAILURE;
    }

    run();

    cleanup();

    return EXIT_SUCCESS;
}

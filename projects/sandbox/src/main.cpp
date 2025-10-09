#include "VkBootstrap.h"
#include "volk.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "fmt/core.h"

#include <fstream>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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

    int current_frame = 0;
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
        .set_app_name("Hello Triangle")
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

    vkb::PhysicalDeviceSelector physical_device_selector(g_instance);
    physical_device_selector
        .set_minimum_version(1, 3)
        .set_surface(g_surface)
        .set_required_features_11({
            .shaderDrawParameters = VK_TRUE,
        })
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
    vkb::SwapchainBuilder swapchain_builder(g_device);
    swapchain_builder
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

    VkPipelineShaderStageCreateInfo vertex_stage = {};
    vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage.module = shader_module;
    vertex_stage.pName = "vertMain";

    VkPipelineShaderStageCreateInfo fragment_stage = {};
    fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage.module = shader_module;
    fragment_stage.pName = "fragMain";

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        vertex_stage,
        fragment_stage,
    };

    VkPipelineVertexInputStateCreateInfo vertex_input = {};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = 0;
    vertex_input.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

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
    pipeline_info.pVertexInputState = &vertex_input;
    pipeline_info.pInputAssemblyState = &input_assembly;
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
draw()
{
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

        g_render_data.current_frame = (g_render_data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

int
init()
{
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

    g_window = SDL_CreateWindow("Hello Triangle", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

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
    SDL_Vulkan_DestroySurface(g_instance, g_surface, nullptr);
    vkb::destroy_instance(g_instance);

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

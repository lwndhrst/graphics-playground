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
    VkQueue present_queue;

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

int
init_vulkan()
{
    if (VK_SUCCESS != volkInitialize())
    {
        fmt::println("Failed to initialize volk");
        return -1;
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
        return -1;
    }

    g_instance = instance_ret.value();

    volkLoadInstanceOnly(g_instance);

    if (!SDL_Vulkan_CreateSurface(g_window, g_instance, nullptr, &g_surface))
    {
        fmt::println("Failed to create surface: {}", SDL_GetError());
        return -1;
    }

    vkb::PhysicalDeviceSelector physical_device_selector(g_instance);
    physical_device_selector
        .set_minimum_version(1, 3)
        .set_surface(g_surface)
        .set_required_features_13({
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        });

    auto physical_device_ret = physical_device_selector.select();
    if (!physical_device_ret.has_value())
    {
        fmt::println("Failed to select physical device: {}", physical_device_ret.error().message());
        return -1;
    }

    fmt::println("Selected physical device: {}", physical_device_ret->name);

    vkb::DeviceBuilder device_builder(physical_device_ret.value());

    auto device_ret = device_builder.build();
    if (!device_ret.has_value())
    {
        fmt::println("Failed to create device: {}", device_ret.error().message());
        return -1;
    }

    g_device = device_ret.value();

    volkLoadDevice(g_device);

    return 0;
}

int
get_queues()
{
    auto graphics_queue_ret = g_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret.has_value())
    {
        fmt::println("Failed to get graphics queue: {}", graphics_queue_ret.error().message());
        return -1;
    }

    g_render_data.graphics_queue = graphics_queue_ret.value();

    auto present_queue_ret = g_device.get_queue(vkb::QueueType::present);
    if (!present_queue_ret.has_value())
    {
        fmt::println("Failed to get present queue: {}", present_queue_ret.error().message());
        return -1;
    }

    g_render_data.present_queue = present_queue_ret.value();

    return 0;
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
        return -1;
    }

    vkb::destroy_swapchain(g_swapchain);

    g_swapchain = swapchain_ret.value();

    return 0;
}

int
get_swapchain_images_and_views()
{
    auto swapchain_images_ret = g_swapchain.get_images();
    if (!swapchain_images_ret.has_value())
    {
        fmt::println("Failed to get swapchain images: {}", swapchain_images_ret.error().message());
        return -1;
    }

    auto swapchain_image_views_ret = g_swapchain.get_image_views();
    if (!swapchain_image_views_ret.has_value())
    {
        fmt::println("Failed to get swapchain image views: {}", swapchain_image_views_ret.error().message());
        return -1;
    }

    g_render_data.swapchain_images = swapchain_images_ret.value();
    g_render_data.swapchain_image_views = swapchain_image_views_ret.value();

    return 0;
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
createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shader_module;
    if (VK_SUCCESS != vkCreateShaderModule(g_device, &create_info, nullptr, &shader_module))
    {
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

int
create_pipeline()
{
    return 0;
}

int
create_command_objects()
{
    return 0;
}

int
create_synchronization_objects()
{
    return 0;
}

int
resize_swapchain()
{
    vkDeviceWaitIdle(g_device);

    g_swapchain.destroy_image_views(g_render_data.swapchain_image_views);

    if (0 != create_swapchain())
        return -1;

    if (0 != get_swapchain_images_and_views())
        return -1;

    if (0 != create_pipeline())
        return -1;

    if (0 != create_command_objects())
        return -1;

    if (0 != create_synchronization_objects())
        return -1;

    return 0;
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
            if (0 != resize_swapchain())
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

    if (0 != init_vulkan())
        return -1;

    if (0 != get_queues())
        return -1;

    if (0 != create_swapchain())
        return -1;

    if (0 != get_swapchain_images_and_views())
        return -1;

    return 0;
}

void
cleanup()
{
    vkDeviceWaitIdle(g_device);

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
    if (0 != init())
    {
        cleanup();
        return -1;
    }

    run();

    cleanup();

    return 0;
}

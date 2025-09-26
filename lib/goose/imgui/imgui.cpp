#include "goose/imgui/imgui.hpp"

#include "goose/common/assert.hpp"
#include "goose/render/context.hpp"
#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"
#include "goose/render/instance.hpp"
#include "goose/window/window.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"

static VkDescriptorPool g_imgui_descriptor_pool = VK_NULL_HANDLE;
static bool g_imgui_initialized = false;

// TODO: Any way to make this work with multiple windows?

bool
goose::init_imgui_internal(const Window &window, const goose::render::RenderContext &ctx)
{
    if (g_imgui_initialized)
    {
        return true;
    }

    const VkDevice &device = goose::render::Device::get();

    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = static_cast<u32>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    VkDescriptorPool descriptor_pool;
    VkResult result = vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &descriptor_pool);

    // TODO: Error handling
    VK_ASSERT(result);

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForVulkan(window.window);

    ImGui_ImplVulkan_InitInfo imgui_vulkan_init_info = {
        .ApiVersion = VK_API_VERSION_1_4,
        .Instance = goose::render::Instance::get(),
        .PhysicalDevice = goose::render::Device::get_physical(),
        .Device = device,
        .Queue = goose::render::Device::get_queue_families().graphics.queues[0],
        .DescriptorPool = descriptor_pool,
        .MinImageCount = 3,
        .ImageCount = 3,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &ctx.swapchain.image_format,
        },
    };

    ImGui_ImplVulkan_Init(&imgui_vulkan_init_info);

    g_imgui_descriptor_pool = descriptor_pool;
    g_imgui_initialized = true;

    return true;
}

void
goose::quit_imgui_internal()
{
    const VkDevice &device = goose::render::Device::get();
    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(device, g_imgui_descriptor_pool, nullptr);

    g_imgui_descriptor_pool = nullptr;
    g_imgui_initialized = false;
}

void
goose::render::draw_imgui(VkCommandBuffer cmd, VkImageView view, VkExtent2D extent)
{
    VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
    };

    VkRect2D render_area = {
        .offset = {0, 0},
        .extent = extent,
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = render_area,
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
    };

    vkCmdBeginRendering(cmd, &rendering_info);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
}

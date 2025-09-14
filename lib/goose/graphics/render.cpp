#include "goose/graphics/render.hpp"

#include "goose/core/util.hpp"

namespace goose::graphics {

// Don't really want these in the header
void select_physical_device();
void create_logical_device();

bool
create_instance(RenderData *data, const char *app_name, u32 app_version)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = app_version;
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_4;

    // TODO: Properly check layer/extension support

#ifdef GOOSE_DEBUG
    data->instance_layers.push_back("VK_LAYER_KHRONOS_validation");
    data->instance_extensions.push_back("VK_EXT_debug_utils");
#endif

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = static_cast<u32>(data->instance_layers.size());
    instance_create_info.ppEnabledLayerNames = data->instance_layers.data();
    instance_create_info.enabledExtensionCount = static_cast<u32>(data->instance_extensions.size());
    instance_create_info.ppEnabledExtensionNames = data->instance_extensions.data();

    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &data->instance);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Vulkan error: {}", string_VkResult(result));
        return false;
    }

    return true;
}

bool
init(RenderData *data, VkSurfaceKHR surface)
{
    data->surface = surface;

    return true;
}

void
cleanup(RenderData *data)
{
    if (data->surface != nullptr)
    {
        vkDestroySurfaceKHR(data->instance, data->surface, nullptr);
    }

    if (data->instance != nullptr)
    {
        vkDestroyInstance(data->instance, nullptr);
    }
}

} // namespace goose::graphics

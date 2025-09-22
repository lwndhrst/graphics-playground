#include "goose/render/instance.hpp"

#include "goose/common/log.hpp"

#include "SDL3/SDL_vulkan.h"

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

static goose::render::Instance s_instance = {};
static bool s_initialized = false;

bool
goose::render::create_instance(const char *app_name, u32 app_version)
{
    if (s_initialized)
    {
        LOG_INFO("Vulkan instance is already initialized");
        return true;
    }

    Instance instance = {};

#ifdef GOOSE_DEBUG
    instance.layers.push_back(VALIDATION_LAYER_NAME);
    instance.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    u32 sdl_instance_extension_count;
    auto sdl_instance_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_instance_extension_count);
    for (usize i = 0; i < sdl_instance_extension_count; ++i)
    {
        instance.extensions.push_back(sdl_instance_extensions[i]);
    }

    instance.extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name,
        .applicationVersion = app_version,
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    // TODO: Properly check layer/extension support

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<u32>(instance.layers.size()),
        .ppEnabledLayerNames = instance.layers.data(),
        .enabledExtensionCount = static_cast<u32>(instance.extensions.size()),
        .ppEnabledExtensionNames = instance.extensions.data(),
    };

    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance.instance);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);
        return false;
    }

    s_instance = std::move(instance);
    s_initialized = true;

    return true;
}

void
goose::render::destroy_instance()
{
    vkDestroyInstance(s_instance.instance, nullptr);

    s_instance = {};
    s_initialized = false;
}

const goose::render::Instance &
goose::render::get_instance()
{
    ASSERT(s_initialized, "Vulkan instance is not initialized");
    return s_instance;
}

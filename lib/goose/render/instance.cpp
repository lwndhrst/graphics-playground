#include "goose/render/instance.hpp"

#include "goose/common/log.hpp"

#include "SDL3/SDL_vulkan.h"

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

bool
goose::render::create_instance(const char *app_name, u32 app_version)
{
    LOG_INFO("Creating vulkan instance");

    if (Instance::s_initialized)
    {
        LOG_INFO("Vulkan instance is already initialized");
        return true;
    }

    std::vector<const char *> layers;
    std::vector<const char *> extensions;

#ifdef GOOSE_DEBUG
    layers.push_back(VALIDATION_LAYER_NAME);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    u32 sdl_instance_extension_count;
    auto sdl_instance_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_instance_extension_count);
    for (usize i = 0; i < sdl_instance_extension_count; ++i)
    {
        extensions.push_back(sdl_instance_extensions[i]);
    }

    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    if (!layers.empty())
    {
        LOG_INFO("Requested vulkan instance layers:");
        for (const auto &l : layers)
        {
            LOG_INFO("- {}", l);
        }
    }

    if (!extensions.empty())
    {
        LOG_INFO("Requested vulkan instance extensions:");
        for (const auto &e : extensions)
        {
            LOG_INFO("- {}", e);
        }
    }

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
        .enabledLayerCount = static_cast<u32>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<u32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance;
    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result);
        return false;
    }

    Instance::s_instance = instance;
    Instance::s_initialized = true;

    LOG_INFO("Vulkan instance created successfully");

    return true;
}

void
goose::render::destroy_instance()
{
    LOG_INFO("Destroying vulkan instance");

    vkDestroyInstance(Instance::s_instance, nullptr);

    Instance::s_instance = nullptr;
    Instance::s_initialized = false;
}

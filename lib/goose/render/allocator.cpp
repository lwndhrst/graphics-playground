#include "goose/render/allocator.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"

// NOTE: VmaAllocator should be thread-safe according to documentation
static VmaAllocator s_allocator = nullptr;
static bool s_initialized = false;

bool
goose::render::create_allocator()
{
    if (s_initialized)
    {
        LOG_INFO("Vulkan memory allocator is already initialized");
        return true;
    }

    VmaAllocator allocator = {};

    const Instance &instance = get_instance();
    const Device &device = get_device();

    VmaAllocatorCreateInfo allocator_create_info = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = device.physical,
        .device = device.logical,
        .instance = instance.instance,
    };

    VkResult result = vmaCreateAllocator(&allocator_create_info, &allocator);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create vulkan memory allocator");
        return false;
    }

    s_allocator = allocator;
    s_initialized = true;

    return true;
}

void
goose::render::destroy_allocator()
{
    vmaDestroyAllocator(s_allocator);

    s_allocator = nullptr;
    s_initialized = false;
}

const VmaAllocator &
goose::render::get_allocator()
{
    ASSERT(s_initialized, "Vulkan memory allocator is not initialized");
    return s_allocator;
}

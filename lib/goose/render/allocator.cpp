#include "goose/render/allocator.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"
#include "goose/render/instance.hpp"

bool
goose::render::create_allocator()
{
    if (Allocator::s_initialized)
    {
        LOG_INFO("Vulkan memory allocator is already initialized");
        return true;
    }

    VmaAllocatorCreateInfo allocator_create_info = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = Device::get_physical(),
        .device = Device::get(),
        .instance = Instance::get(),
    };

    VmaAllocator allocator;
    VkResult result = vmaCreateAllocator(&allocator_create_info, &allocator);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create vulkan memory allocator");
        return false;
    }

    Allocator::s_allocator = allocator;
    Allocator::s_initialized = true;

    return true;
}

void
goose::render::destroy_allocator()
{
    vmaDestroyAllocator(Allocator::s_allocator);

    Allocator::s_allocator = nullptr;
    Allocator::s_initialized = false;
}

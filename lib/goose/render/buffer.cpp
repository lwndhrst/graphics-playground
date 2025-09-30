#include "goose/render/buffer.hpp"

#include "goose/common/assert.hpp"
#include "goose/common/log.hpp"
#include "goose/render/device.hpp"

goose::render::BufferBuilder::BufferBuilder()
{
    // TODO: Reasonable defaults?
    _usage_flags = 0;
    _memory_usage = MEMORY_USAGE_GPU_ONLY;
    _device_address = false;
}

goose::render::BufferBuilder &
goose::render::BufferBuilder::set_usage_flags(VkBufferUsageFlags usage_flags)
{
    _usage_flags = usage_flags;

    return *this;
}

goose::render::BufferBuilder &
goose::render::BufferBuilder::set_memory_usage(MemoryUsage memory_usage)
{
    _memory_usage = memory_usage;

    return *this;
}

goose::render::BufferBuilder &
goose::render::BufferBuilder::enable_device_address()
{
    _device_address = true;

    return *this;
}

goose::render::BufferBuilder &
goose::render::BufferBuilder::disable_device_address()
{
    _device_address = false;

    return *this;
}

bool
goose::render::BufferBuilder::build(BufferInfo &buffer, u32 size)
{
    VkBufferUsageFlags usage_flags = _usage_flags;

    if (_device_address)
    {
        usage_flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags,
    };

    VmaAllocationCreateInfo buffer_allocation_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };

    switch (_memory_usage)
    {
    case MEMORY_USAGE_GPU_ONLY:
        buffer_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case MEMORY_USAGE_CPU_ONLY:
        buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        break;
    case MEMORY_USAGE_CPU_TO_GPU:
        buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case MEMORY_USAGE_GPU_TO_CPU:
        buffer_allocation_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        break;
    default:
        LOG_ERROR("Missing or invalid memory usage flags");
        return false;
    }

    VkResult result = vmaCreateBuffer(
        Allocator::get(),
        &buffer_create_info,
        &buffer_allocation_info,
        &buffer.buffer,
        &buffer.allocation,
        &buffer.allocation_info);

    // TODO: Error handling
    VK_ASSERT(result);

    if (!_device_address)
    {
        buffer.device_address.reset();
        return true;
    }

    VkBufferDeviceAddressInfo device_address_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer.buffer,
    };

    buffer.device_address = vkGetBufferDeviceAddress(Device::get(), &device_address_info);

    return true;
}

void
goose::render::destroy_buffer(BufferInfo &buffer)
{
    vmaDestroyBuffer(Allocator::get(), buffer.buffer, buffer.allocation);

    buffer = {};
}

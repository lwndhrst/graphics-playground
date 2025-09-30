#pragma once

#include "goose/common/types.hpp"
#include "goose/render/allocator.hpp"

namespace goose::render {

struct BufferInfo {
    VkBuffer buffer;
    std::optional<VkDeviceAddress> device_address;

    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
};

struct BufferBuilder {
    VkBufferUsageFlags _usage_flags;
    MemoryUsage _memory_usage;
    bool _device_address;

    BufferBuilder();

    BufferBuilder &set_usage_flags(VkBufferUsageFlags usage_flags);
    BufferBuilder &set_memory_usage(MemoryUsage memory_usage);

    BufferBuilder &enable_device_address();
    BufferBuilder &disable_device_address();

    bool build(BufferInfo &buffer, u32 size);
};

void destroy_buffer(BufferInfo &buffer);

} // namespace goose::render

#pragma once

#include "goose/common/assert.hpp"
#include "goose/common/types.hpp"

namespace goose::render {

enum MemoryUsage {
    MEMORY_USAGE_GPU_ONLY,
};

struct Allocator {
    inline static bool s_initialized;

    inline static VmaAllocator s_allocator;

    static const VmaAllocator &get()
    {
        ASSERT(s_initialized, "Vulkan memory allocator is not initialized");
        return s_allocator;
    }
};

bool create_allocator();
void destroy_allocator();

} // namespace goose::render

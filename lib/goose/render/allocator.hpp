#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

enum MemoryUsage {
    MEMORY_USAGE_GPU_ONLY,
};

bool create_allocator();
void destroy_allocator();

const VmaAllocator &get_allocator();

} // namespace goose::render

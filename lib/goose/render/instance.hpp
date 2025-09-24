#pragma once

#include "goose/common/assert.hpp"
#include "goose/common/types.hpp"

namespace goose::render {

// TODO: Setup debug messenger?

struct Instance {
    inline static bool s_initialized;

    inline static VkInstance s_instance;

    static const VkInstance &get()
    {
        ASSERT(s_initialized, "Vulkan instance is not initialized");
        return s_instance;
    };
};

bool create_instance(const char *app_name, u32 app_version);
void destroy_instance();

} // namespace goose::render

#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct Instance {
    VkInstance handle;
    std::vector<const char *> layers;
    std::vector<const char *> extensions;
};

bool create_instance(const char *app_name, u32 app_version);
Instance get_instance();
void destroy_instance();

// TODO: Setup debug messenger?

} // namespace goose::render

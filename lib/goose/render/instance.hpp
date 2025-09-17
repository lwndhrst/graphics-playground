#pragma once

#include "goose/core/types.hpp"

namespace goose::render {

VkInstance create_instance(const char *app_name, u32 app_version, const std::vector<const char *> &layers, const std::vector<const char *> &extensions);

// TODO: Setup debug messenger?

} // namespace goose::render

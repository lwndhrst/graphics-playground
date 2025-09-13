#pragma once

#include "goose/core/types.hpp"

#define LOG_DEBUG(...) \
    fmt::println("DEBUG {}", fmt::format(__VA_ARGS__))

#define LOG_WARN(...) \
    fmt::println("WARN  {}", fmt::format(__VA_ARGS__))

#define LOG_ERROR(...) \
    fmt::println("ERROR {}", fmt::format(__VA_ARGS__))

#define LOG_VK_ERROR(x)                                      \
    VkResult err = x;                                        \
    if (err)                                                 \
    {                                                        \
        LOG_ERROR("Vulkan error: {}", string_VkResult(err)); \
    }

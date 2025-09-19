#pragma once

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

#define DO_NOTHING() \
    do               \
    {                \
    } while (0)

#include "fmt/core.h"
#include "vulkan/vk_enum_string_helper.h"

#ifdef GOOSE_DEBUG
#define LOG_DEBUG(...) \
    fmt::println(" DEBUG | {}", fmt::format(__VA_ARGS__))
#define LOG_INFO(...) \
    fmt::println("  INFO | {}", fmt::format(__VA_ARGS__))
#define LOG_WARN(...) \
    fmt::println("  WARN | {}", fmt::format(__VA_ARGS__))
#define LOG_ERROR(...) \
    fmt::println(" ERROR | {}", fmt::format(__VA_ARGS__))
#else
#define LOG_DEBUG(...) \
    DO_NOTHING()
#define LOG_INFO(...) \
    DO_NOTHING()
#define LOG_WARN(...) \
    DO_NOTHING()
#define LOG_ERROR(...) \
    DO_NOTHING()
#endif

#define VK_LOG_ERROR(result) \
    LOG_ERROR("Vulkan: {}", string_VkResult(result))

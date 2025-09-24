#pragma once

#include "fmt/core.h"
#include "vulkan/vk_enum_string_helper.h"

#define DO_NOTHING() \
    do               \
    {                \
    } while (false)

#ifdef GOOSE_DEBUG
#define LOG_DEBUG(...) \
    fmt::println("DEBUG | {}", fmt::format(__VA_ARGS__))
#else
#define LOG_DEBUG(...) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define LOG_INFO(...) \
    fmt::println("INFO  | {}", fmt::format(__VA_ARGS__))
#else
#define LOG_INFO(...) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define LOG_WARN(...) \
    fmt::println("WARN  | {}", fmt::format(__VA_ARGS__))
#else
#define LOG_WARN(...) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define LOG_ERROR(...) \
    fmt::println("ERROR | {}", fmt::format(__VA_ARGS__))
#else
#define LOG_ERROR(...) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define VK_LOG_ERROR(result) \
    LOG_ERROR("Vulkan: {}", string_VkResult(result))
#else
#define VK_LOG_ERROR(result) \
    DO_NOTHING()
#endif

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
#define ASSERT(condition, message)                                                                            \
    do                                                                                                        \
    {                                                                                                         \
        if (!(condition))                                                                                     \
        {                                                                                                     \
            LOG_ERROR("Assertion '" #condition "' failed in {} at line {}: {}", __FILE__, __LINE__, message); \
            std::abort();                                                                                     \
        }                                                                                                     \
    } while (false)

#else
#define ASSERT(condition, msg) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define VK_ASSERT(x)                                                                                                                 \
    do                                                                                                                               \
    {                                                                                                                                \
        VkResult the_sea_of_suffering_has_no_bounds = x;                                                                             \
        if (the_sea_of_suffering_has_no_bounds != VK_SUCCESS)                                                                        \
        {                                                                                                                            \
            LOG_ERROR("Vulkan error in {} at line {}: {}", __FILE__, __LINE__, string_VkResult(the_sea_of_suffering_has_no_bounds)); \
            std::abort();                                                                                                            \
        }                                                                                                                            \
    } while (false)

#else
#define VK_ASSERT(x) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define VK_LOG_ERROR(result) \
    LOG_ERROR("Vulkan: {}", string_VkResult(result))
#else
#define VK_LOG_ERROR(result) \
    DO_NOTHING()
#endif

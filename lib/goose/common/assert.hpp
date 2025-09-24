#pragma once

#include "goose/common/log.hpp"

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
#define ASSERT(condition, message) \
    DO_NOTHING()
#endif

#ifdef GOOSE_DEBUG
#define VK_ASSERT(x)                                                                                                                \
    do                                                                                                                              \
    {                                                                                                                               \
        VkResult the_sea_of_suffering_is_boundless = x;                                                                             \
        if (the_sea_of_suffering_is_boundless != VK_SUCCESS)                                                                        \
        {                                                                                                                           \
            LOG_ERROR("Vulkan error in {} at line {}: {}", __FILE__, __LINE__, string_VkResult(the_sea_of_suffering_is_boundless)); \
            std::abort();                                                                                                           \
        }                                                                                                                           \
    } while (false)

#else
#define VK_ASSERT(x) \
    DO_NOTHING()
#endif

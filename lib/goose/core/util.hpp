#pragma once

#define DO_NOTHING() \
    do               \
    {                \
    } while (false)

#include "fmt/color.h"
#include "fmt/core.h"
#include "vulkan/vk_enum_string_helper.h"

// TODO: Less formatting shenanigans?
#ifdef GOOSE_DEBUG
#define LOG_DEBUG(...) \
    fmt::println("{} {}", "DEBUG", fmt::format(__VA_ARGS__));
#define LOG_WARN(...) \
    fmt::println("{} {}", fmt::format(fg(fmt::color::yellow), "WARN "), fmt::format(fg(fmt::color::yellow), __VA_ARGS__));
#define LOG_ERROR(...) \
    fmt::println("{} {}", fmt::format(fg(fmt::color::red), "ERROR"), fmt::format(fg(fmt::color::red), __VA_ARGS__));
#else
#define LOG_DEBUG(...) \
    DO_NOTHING()
#define LOG_WARN(...) \
    DO_NOTHING()
#define LOG_ERROR(...) \
    DO_NOTHING()
#endif

#pragma once

#include <cstdio>
#include <format>

#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RESET "\033[0m"

// you can pass LOG_LEVEL to clang++ using the -D flag
#ifndef LOG_LEVEL
#define LOG_LEVEL NONE
#endif

namespace gp::log {

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE,
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"

template <typename T>
static void
debug(T msg)
{
    if (LogLevel::LOG_LEVEL > LogLevel::DEBUG)
        return;

    fprintf(stdout, "DEBUG   %s\n", std::format("{}", msg).c_str());
}

template <typename T>
static void
info(T msg)
{
    if (LogLevel::LOG_LEVEL > LogLevel::INFO)
        return;

    fprintf(stdout, "INFO    %s\n", std::format("{}", msg).c_str());
}

template <typename T>
static void
warn(T msg)
{
    if (LogLevel::LOG_LEVEL > LogLevel::WARN)
        return;

    fprintf(stdout, "%sWARN    %s%s\n", ANSI_YELLOW, std::format("{}", msg).c_str(), ANSI_RESET);
}

template <typename T>
static void
error(T msg)
{
    if (LogLevel::LOG_LEVEL > LogLevel::ERROR)
        return;

    fprintf(stderr, "%sERROR   %s%s\n", ANSI_RED, std::format("{}", msg).c_str(), ANSI_RESET);
}

#pragma clang diagnostic pop

} // namespace gp::log

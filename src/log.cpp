#include "log.h"

#include <cstdio>

#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RESET "\033[0m"

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO
#endif

namespace gp::log {

enum LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  NONE,
};

static LogLevel level = LogLevel::LOG_LEVEL;

void debug(std::string &&msg) {
  if (level > LogLevel::DEBUG) return;
  fprintf(stdout, "DEBUG   %s\n", msg.c_str());
}

void info(std::string &&msg) {
  if (level > LogLevel::INFO) return;
  fprintf(stdout, "INFO    %s\n", msg.c_str());
}

void warn(std::string &&msg) {
  if (level > LogLevel::WARN) return;
  fprintf(stdout, "%sWARN    %s%s\n", ANSI_YELLOW, msg.c_str(), ANSI_RESET);
}

void error(std::string &&msg) {
  if (level > LogLevel::ERROR) return;
  fprintf(stderr, "%sERROR   %s%s\n", ANSI_RED, msg.c_str(), ANSI_RESET);
}

} // namespace gp::log

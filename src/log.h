#pragma once

#include <string>

namespace gp::log {

void debug(std::string &&msg);
void info(std::string &&msg);
void warn(std::string &&msg);
void error(std::string &&msg);

} // namespace gp::log
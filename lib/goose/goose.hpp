#pragma once

#include "goose/common/types.hpp"

namespace goose {

bool init(const char *app_name);
void quit();

bool create_window(const char *title, u32 width, u32 height);
bool should_run();

} // namespace goose

namespace goose::render {

void begin_frame();
void end_frame();

} // namespace goose::render

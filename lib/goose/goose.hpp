#pragma once

#include "goose/core/types.hpp"

namespace goose {

bool init(const char *app_name, u32 app_version);

bool create_window(const char *title, u32 width, u32 height);
bool window_should_close();

void update();

void quit();

} // namespace goose

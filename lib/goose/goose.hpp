#pragma once

#include "goose/common/types.hpp"

namespace goose {

bool init(const char *app_name);

bool create_window(const char *title, u32 width, u32 height);

bool window_should_close();

void quit();

} // namespace goose

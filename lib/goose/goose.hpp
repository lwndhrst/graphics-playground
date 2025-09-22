#pragma once

#include "goose/common/log.hpp"
#include "goose/common/types.hpp"
#include "goose/render/context.hpp"
#include "goose/render/util.hpp"
#include "goose/window/window.hpp"

namespace goose {

bool init(const char *app_name);
void quit();

bool should_run();

} // namespace goose

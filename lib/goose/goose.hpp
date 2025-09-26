#pragma once

#include "goose/common/log.hpp"
#include "goose/common/types.hpp"
#include "goose/common/util.hpp"
#include "goose/render/allocator.hpp"
#include "goose/render/context.hpp"
#include "goose/render/descriptors.hpp"
#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"
#include "goose/render/image.hpp"
#include "goose/window/window.hpp"

namespace goose {

bool init(const char *app_name);
void quit();

void init_imgui(const WindowInfo &window, const goose::render::RenderContext &ctx);
void quit_imgui();

bool should_run();

} // namespace goose

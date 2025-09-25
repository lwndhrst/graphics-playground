#pragma once

#include "goose/common/types.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

namespace goose {

bool init_imgui_internal(const Window &window, const goose::render::RenderContext &ctx);
void quit_imgui_internal();

} // namespace goose

namespace goose::render {

void draw_imgui(VkCommandBuffer cmd, VkImageView view, VkExtent2D extent, VkImageLayout layout);

} // namespace goose::render

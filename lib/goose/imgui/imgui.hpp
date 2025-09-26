#pragma once

#include "goose/common/types.hpp"

#include "imgui.h"

namespace goose {

bool init_imgui_internal(const WindowInfo &window, const goose::render::RenderContext &ctx);
void quit_imgui_internal();

} // namespace goose

namespace goose::render {

// NOTE: Image has to be in VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL before calling
void draw_imgui(VkCommandBuffer cmd, VkImageView view, VkExtent2D extent);

} // namespace goose::render

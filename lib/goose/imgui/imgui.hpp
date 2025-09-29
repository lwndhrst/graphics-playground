#pragma once

#include "goose/common/types.hpp"

#include "imgui.h"

namespace goose::imgui {

bool init_imgui(const WindowInfo &window, const goose::render::RenderContext &ctx);
void quit_imgui();

// NOTE: Image has to be in VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL before calling
void draw_imgui(VkCommandBuffer cmd, VkImageView view, VkExtent2D extent);

} // namespace goose::imgui

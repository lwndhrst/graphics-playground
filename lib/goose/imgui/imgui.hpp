#pragma once

#include "goose/common/types.hpp"

#include "goose/render/helpers.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

namespace goose {

bool init_imgui_internal(const Window &window, const goose::render::RenderContext &ctx);
void quit_imgui_internal();

} // namespace goose

namespace goose::render {

inline void
draw_imgui(VkCommandBuffer cmd, VkImageView view, VkExtent2D extent, VkImageLayout layout)
{
    VkRenderingAttachmentInfo color_attachment =
        goose::render::make_rendering_attachment_info(view, nullptr, layout);

    VkRenderingInfo rendering_info =
        goose::render::make_rendering_info(extent, &color_attachment, nullptr);

    vkCmdBeginRendering(cmd, &rendering_info);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
}

} // namespace goose::render

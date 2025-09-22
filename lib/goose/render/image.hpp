#pragma once

#include "goose/common/types.hpp"
#include "goose/render/allocator.hpp"

namespace goose::render {

struct Image {
    VkImage image;
    VkImageView view;

    VkExtent3D extent;
    VkFormat format;

    VmaAllocation allocation;
};

bool create_image(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage_flags, VkImageAspectFlags aspect_flags, MemoryUsage memory_usage, Image &image);
void destroy_image(Image &image);

// NOTE: Used as target image during render loop
bool create_draw_image(VkExtent2D extent, Image &image);

} // namespace goose::render

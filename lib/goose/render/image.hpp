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

bool create_image(Image &image, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage_flags, VkImageAspectFlags aspect_flags, MemoryUsage memory_usage);
void destroy_image(Image &image);

struct ImageBuilder {
    struct {
        VkExtent3D extent;
        VkFormat format;

        VkImageUsageFlags usage_flags;
        VkImageAspectFlags aspect_flags;

        MemoryUsage memory_usage;
    } properties;

    ImageBuilder();

    ImageBuilder &set_extent(const VkExtent2D &extent);
    ImageBuilder &set_extent(const VkExtent3D &extent);
    ImageBuilder &set_format(const VkFormat &format);
    ImageBuilder &add_usage_flags(const VkImageUsageFlags &usage_flags);
    ImageBuilder &remove_usage_flags(const VkImageUsageFlags &usage_flags);
    ImageBuilder &add_aspect_flags(const VkImageAspectFlags &aspect_flags);
    ImageBuilder &remove_aspect_flags(const VkImageAspectFlags &aspect_flags);
    ImageBuilder &set_memory_usage(const MemoryUsage &memory_usage);

    bool build(Image &image);
};

} // namespace goose::render

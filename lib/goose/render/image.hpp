#pragma once

#include "goose/common/types.hpp"
#include "goose/render/allocator.hpp"

namespace goose::render {

enum ImageType {
    IMAGE_TYPE_2D,
    IMAGE_TYPE_3D,
};

struct ImageInfo {
    VkImage image;
    VkImageView view;

    VkFormat format;

    union {
        VkExtent3D extent;
        VkExtent2D extent_2d;
    };

    VmaAllocation allocation;
};

struct ImageBuilder {
    VkImageType _image_type;
    VkImageViewType _image_view_type;
    VkFormat _format;
    VkExtent3D _extent;
    u32 _mip_levels;
    u32 _array_layers;
    VkSampleCountFlagBits _samples;
    VkImageTiling _tiling;
    VkImageUsageFlags _usage_flags;
    VkImageAspectFlags _aspect_flags;
    MemoryUsage _memory_usage;

    ImageBuilder(ImageType type);

    ImageBuilder &set_format(VkFormat format);
    ImageBuilder &set_extent(VkExtent2D extent);
    ImageBuilder &set_extent(VkExtent3D extent);
    ImageBuilder &set_mip_levels(u32 mip_levels);
    ImageBuilder &set_array_layers(u32 array_layers);
    ImageBuilder &set_samples(VkSampleCountFlagBits samples);
    ImageBuilder &set_tiling(VkImageTiling tiling);
    ImageBuilder &set_usage_flags(VkImageUsageFlags usage_flags);
    ImageBuilder &set_aspect_flags(VkImageAspectFlags aspect_flags);
    ImageBuilder &set_memory_usage(MemoryUsage memory_usage);

    bool build(ImageInfo &image);
};

void destroy_image(ImageInfo &image);

} // namespace goose::render

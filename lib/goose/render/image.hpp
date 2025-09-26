#pragma once

#include "goose/common/types.hpp"
#include "goose/render/allocator.hpp"

namespace goose::render {

enum ImageType {
    IMAGE_TYPE_2D,
};

struct ImageInfo {
    VkImage image;
    VkImageView view;

    VkFormat format;
    VkExtent3D extent;

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

    ImageBuilder(const ImageType &type);

    ImageBuilder &set_type(const ImageType &type);
    ImageBuilder &set_format(const VkFormat &format);
    ImageBuilder &set_extent(const VkExtent3D &extent);
    ImageBuilder &set_mip_levels(const u32 &mip_levels);
    ImageBuilder &set_array_layers(const u32 &array_layers);
    ImageBuilder &set_samples(const VkSampleCountFlagBits &samples);
    ImageBuilder &set_tiling(const VkImageTiling &tiling);
    ImageBuilder &set_usage_flags(const VkImageUsageFlags &usage_flags);
    ImageBuilder &set_aspect_flags(const VkImageAspectFlags &aspect_flags);
    ImageBuilder &set_memory_usage(const MemoryUsage &memory_usage);

    bool build(ImageInfo &image);
};

void destroy_image(ImageInfo &image);

} // namespace goose::render

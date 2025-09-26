#include "goose/render/image.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_type(const ImageType &type)
{
    switch (type)
    {
    case goose::render::IMAGE_TYPE_2D:
        _image_type = VK_IMAGE_TYPE_2D;
        _image_view_type = VK_IMAGE_VIEW_TYPE_2D;
        break;
    default:
        LOG_ERROR("Unsupported image type");
        break;
    }

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_format(const VkFormat &format)
{
    _format = format;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_extent(const VkExtent3D &extent)
{
    _extent = extent;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_mip_levels(const u32 &mip_levels)
{
    _mip_levels = mip_levels;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_array_layers(const u32 &array_layers)
{
    _array_layers = array_layers;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_samples(const VkSampleCountFlagBits &samples)
{
    _samples = samples;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_tiling(const VkImageTiling &tiling)
{
    _tiling = tiling;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::add_usage_flags(const VkImageUsageFlags &usage_flags)
{
    _usage_flags |= usage_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::remove_usage_flags(const VkImageUsageFlags &usage_flags)
{
    _usage_flags ^= _usage_flags & usage_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::add_aspect_flags(const VkImageAspectFlags &aspect_flags)
{
    _aspect_flags |= aspect_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::remove_aspect_flags(const VkImageAspectFlags &aspect_flags)
{
    _aspect_flags ^= _aspect_flags & aspect_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_memory_usage(const MemoryUsage &memory_usage)
{
    _memory_usage = memory_usage;

    return *this;
}

bool
goose::render::ImageBuilder::build(Image &image)
{
    VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = _image_type,
        .format = _format,
        .extent = _extent,
        .mipLevels = _mip_levels,
        .arrayLayers = _array_layers,
        .samples = _samples,
        .tiling = _tiling,
        .usage = _usage_flags,
    };

    // TODO: Memory usages
    VmaAllocationCreateInfo image_allocation_info = {};
    switch (_memory_usage)
    {
    case MEMORY_USAGE_GPU_ONLY:
        image_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        image_allocation_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    default:
        LOG_ERROR("Missing or unsupported memory usage flags");
        return false;
    }

    vmaCreateImage(Allocator::get(), &image_create_info, &image_allocation_info, &image.image, &image.allocation, nullptr);

    VkImageViewCreateInfo image_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.image,
        .viewType = _image_view_type,
        .format = _format,
        .subresourceRange = {
            .aspectMask = _aspect_flags,
            .baseMipLevel = 0,
            .levelCount = _mip_levels,
            .baseArrayLayer = 0,
            .layerCount = _array_layers,
        },
    };

    VkResult result = vkCreateImageView(Device::get(), &image_view_create_info, nullptr, &image.view);

    // TODO: Error handling
    VK_ASSERT(result);

    image.format = _format;
    image.extent = _extent;

    return true;
}

goose::render::ImageBuilder
goose::render::Image::builder(ImageType type)
{
    // TODO: Reasonable defaults?
    ImageBuilder builder = {
        ._format = VK_FORMAT_R8G8B8A8_SRGB,
        ._extent = {1, 1, 1},
        ._mip_levels = 1,
        ._array_layers = 1,
        ._samples = VK_SAMPLE_COUNT_1_BIT,
        ._tiling = VK_IMAGE_TILING_OPTIMAL,
        ._usage_flags = 0,
        ._aspect_flags = 0,
        ._memory_usage = MEMORY_USAGE_GPU_ONLY,
    };

    builder.set_type(type);

    return builder;
}

void
goose::render::destroy_image(Image &image)
{
    vkDestroyImageView(Device::get(), image.view, nullptr);
    vmaDestroyImage(Allocator::get(), image.image, image.allocation);

    image = {};
}

#include "goose/render/image.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"
#include "goose/render/util.hpp"

bool
goose::render::create_image(
    Image &image,
    VkExtent3D extent,
    VkFormat format,
    VkImageUsageFlags usage_flags,
    VkImageAspectFlags aspect_flags,
    MemoryUsage memory_usage)
{
    const Device &device = get_device();
    const VmaAllocator &allocator = get_allocator();

    VkImageCreateInfo image_create_info = make_image_create_info(format, usage_flags, extent);

    // TODO: Memory usages
    VmaAllocationCreateInfo image_allocation_info = {};
    switch (memory_usage)
    {
    case MEMORY_USAGE_GPU_ONLY:
        image_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        image_allocation_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        break;
    default:
        LOG_ERROR("Missing memory usage flags");
        return false;
    }

    vmaCreateImage(allocator, &image_create_info, &image_allocation_info, &image.image, &image.allocation, nullptr);

    VkImageViewCreateInfo image_view_create_info = make_image_view_create_info(format, image.image, aspect_flags);

    VkResult result = vkCreateImageView(device.logical, &image_view_create_info, nullptr, &image.view);

    // TODO: Error handling
    VK_ASSERT(result);

    return true;
}

void
goose::render::destroy_image(Image &image)
{
    const Device &device = get_device();
    const VmaAllocator &allocator = get_allocator();

    vkDestroyImageView(device.logical, image.view, nullptr);
    vmaDestroyImage(allocator, image.image, image.allocation);

    image = {};
}

goose::render::ImageBuilder::ImageBuilder()
{
    // TODO: Reasonable defaults?
    properties = {
        .extent = {1, 1, 1},
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .usage_flags = 0,
        .aspect_flags = 0,
        .memory_usage = MEMORY_USAGE_GPU_ONLY,
    };
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_extent(const VkExtent2D &extent)
{
    properties.extent.width = extent.width;
    properties.extent.height = extent.width;
    properties.extent.depth = 1;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_extent(const VkExtent3D &extent)
{
    properties.extent = extent;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_format(const VkFormat &format)
{
    properties.format = format;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::add_usage_flags(const VkImageUsageFlags &usage_flags)
{
    properties.usage_flags |= usage_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::remove_usage_flags(const VkImageUsageFlags &usage_flags)
{
    properties.usage_flags ^= properties.usage_flags & usage_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::add_aspect_flags(const VkImageAspectFlags &aspect_flags)
{
    properties.aspect_flags |= aspect_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::remove_aspect_flags(const VkImageAspectFlags &aspect_flags)
{
    properties.aspect_flags ^= properties.aspect_flags & aspect_flags;

    return *this;
}

goose::render::ImageBuilder &
goose::render::ImageBuilder::set_memory_usage(const MemoryUsage &memory_usage)
{
    properties.memory_usage = memory_usage;

    return *this;
}

bool
goose::render::ImageBuilder::build(Image &image)
{
    return create_image(
        image,
        properties.extent,
        properties.format,
        properties.usage_flags,
        properties.aspect_flags,
        properties.memory_usage);
}

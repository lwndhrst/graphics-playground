#include "goose/render/image.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"
#include "goose/render/util.hpp"

bool
goose::render::create_image(
    VkExtent3D extent,
    VkFormat format,
    VkImageUsageFlags usage_flags,
    VkImageAspectFlags aspect_flags,
    MemoryUsage memory_usage,
    Image &image)
{
    VkImageCreateInfo image_create_info = make_image_create_info(format, usage_flags, extent);

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

    vmaCreateImage(get_allocator(), &image_create_info, &image_allocation_info, &image.image, &image.allocation, nullptr);

    VkImageViewCreateInfo image_view_create_info = make_image_view_create_info(format, image.image, aspect_flags);

    VkResult result = vkCreateImageView(get_device().logical, &image_view_create_info, nullptr, &image.view);

    // TODO: Error handling
    VK_ASSERT(result);

    return true;
}

void
goose::render::destroy_image(Image &image)
{
    vkDestroyImageView(get_device().logical, image.view, nullptr);
    vmaDestroyImage(get_allocator(), image.image, image.allocation);
}

bool
goose::render::create_draw_image(VkExtent2D extent, Image &image)
{
    VkExtent3D draw_image_extent = {
        .width = extent.width,
        .height = extent.width,
        .depth = 1,
    };

    VkImageUsageFlags draw_image_usages = 0;
    draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return create_image(
        draw_image_extent,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        draw_image_usages,
        VK_IMAGE_ASPECT_COLOR_BIT,
        MEMORY_USAGE_GPU_ONLY,
        image);
}

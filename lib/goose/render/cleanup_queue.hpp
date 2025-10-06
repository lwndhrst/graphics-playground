#pragma once

#include "goose/common/types.hpp"
#include "goose/render/buffer.hpp"
#include "goose/render/descriptors.hpp"
#include "goose/render/helpers.hpp"
#include "goose/render/image.hpp"
#include "goose/render/pipeline.hpp"

namespace goose::render {

enum CleanupQueueItemType {
    CLEANUP_QUEUE_ITEM_TYPE_DESTROY_IMAGE,
    CLEANUP_QUEUE_ITEM_TYPE_DESTROY_BUFFER,
    CLEANUP_QUEUE_ITEM_TYPE_DESTROY_DESCRIPTOR_POOL,
    CLEANUP_QUEUE_ITEM_TYPE_DESTROY_DESCRIPTOR_SET_LAYOUT,
    CLEANUP_QUEUE_ITEM_TYPE_DESTROY_PIPELINE,
    CLEANUP_QUEUE_ITEM_TYPE_DESTROY_PIPELINE_LAYOUT,
};

struct CleanupQueueItem {
    CleanupQueueItemType type;

    union {
        goose::render::ImageInfo *image;
        goose::render::BufferInfo *buffer;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;
    };
};

struct CleanupQueue {
    std::vector<CleanupQueueItem> items;

    void push(CleanupQueueItem item);
    void clear(bool reverse = false);
};

inline void
CleanupQueue::push(CleanupQueueItem item)
{
    items.push_back(item);
}

inline void
CleanupQueue::clear(bool reverse)
{
    const auto &cleanup = [](CleanupQueueItem &item) {
        switch (item.type)
        {
        case CLEANUP_QUEUE_ITEM_TYPE_DESTROY_IMAGE:
            destroy_image(*item.image);
            break;
        case CLEANUP_QUEUE_ITEM_TYPE_DESTROY_BUFFER:
            destroy_buffer(*item.buffer);
            break;
        case CLEANUP_QUEUE_ITEM_TYPE_DESTROY_DESCRIPTOR_POOL:
            destroy_descriptor_pool(item.descriptor_pool);
            break;
        case CLEANUP_QUEUE_ITEM_TYPE_DESTROY_DESCRIPTOR_SET_LAYOUT:
            destroy_descriptor_set_layout(item.descriptor_set_layout);
            break;
        case CLEANUP_QUEUE_ITEM_TYPE_DESTROY_PIPELINE:
            destroy_pipeline(item.pipeline);
            break;
        case CLEANUP_QUEUE_ITEM_TYPE_DESTROY_PIPELINE_LAYOUT:
            destroy_pipeline_layout(item.pipeline_layout);
            break;
        default:
            ASSERT(false, "unreachable");
            break;
        }
    };

    if (reverse)
    {
        for (auto item = items.rbegin(); item != items.rend(); ++item)
        {
            cleanup(*item);
        }
    }

    else
    {
        for (auto item = items.begin(); item != items.end(); ++item)
        {
            cleanup(*item);
        }
    }

    items.clear();
}

} // namespace goose::render

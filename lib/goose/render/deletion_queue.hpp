#pragma once

#include "goose/common/types.hpp"
#include "goose/render/buffer.hpp"
#include "goose/render/descriptors.hpp"
#include "goose/render/helpers.hpp"
#include "goose/render/image.hpp"
#include "goose/render/pipeline.hpp"

namespace goose::render {

enum DeletionQueueItemType {
    DELETION_QUEUE_ITEM_TYPE_IMAGE,
    DELETION_QUEUE_ITEM_TYPE_BUFFER,
    DELETION_QUEUE_ITEM_TYPE_DESCRIPTOR_POOL,
    DELETION_QUEUE_ITEM_TYPE_DESCRIPTOR_SET_LAYOUT,
    DELETION_QUEUE_ITEM_TYPE_PIPELINE,
    DELETION_QUEUE_ITEM_TYPE_PIPELINE_LAYOUT,
};

struct DeletionQueueItem {
    DeletionQueueItemType type;

    union {
        goose::render::ImageInfo *image;
        goose::render::BufferInfo *buffer;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;
    };
};

struct DeletionQueue {
    std::vector<DeletionQueueItem> items;
};

inline void
deletion_queue_add(DeletionQueue &queue, DeletionQueueItem item)
{
    queue.items.push_back(item);
}

template <typename Iterator>
inline void
deletion_queue_clear(DeletionQueue &queue, bool reverse = false)
{
    const auto &cleanup_item = [](DeletionQueueItem &item) {
        switch (item.type)
        {
        case DELETION_QUEUE_ITEM_TYPE_IMAGE:
            destroy_image(*item.image);
            break;
        case DELETION_QUEUE_ITEM_TYPE_BUFFER:
            destroy_buffer(*item.buffer);
            break;
        case DELETION_QUEUE_ITEM_TYPE_DESCRIPTOR_POOL:
            destroy_descriptor_pool(item.descriptor_pool);
            break;
        case DELETION_QUEUE_ITEM_TYPE_DESCRIPTOR_SET_LAYOUT:
            destroy_descriptor_set_layout(item.descriptor_set_layout);
            break;
        case DELETION_QUEUE_ITEM_TYPE_PIPELINE:
            destroy_pipeline(item.pipeline);
            break;
        case DELETION_QUEUE_ITEM_TYPE_PIPELINE_LAYOUT:
            destroy_pipeline_layout(item.pipeline_layout);
            break;
        default:
            ASSERT(false, "unreachable");
            break;
        }
    };

    if (reverse)
    {
        for (auto item = queue.items.rbegin(); item != queue.items.rend(); ++item)
        {
            cleanup_item(*item);
        }
    }

    else
    {
        for (auto item = queue.items.begin(); item != queue.items.end(); ++item)
        {
            cleanup_item(*item);
        }
    }

    queue.items.clear();
}

} // namespace goose::render

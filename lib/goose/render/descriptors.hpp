#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct DescriptorSetLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    VkShaderStageFlags _stage_flags;
    VkDescriptorSetLayoutCreateFlags _create_flags;

    DescriptorSetLayoutBuilder &add_binding(u32 binding, VkDescriptorType type);
    DescriptorSetLayoutBuilder &set_stage_flags(VkShaderStageFlags stage_flags);
    DescriptorSetLayoutBuilder &set_create_flags(VkDescriptorSetLayoutCreateFlags create_flags);

    bool build(VkDescriptorSetLayout &layout);
};

void destroy_descriptor_set_layout(VkDescriptorSetLayout layout);

} // namespace goose::render

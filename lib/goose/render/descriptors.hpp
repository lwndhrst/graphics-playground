#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct DescriptorSetLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> _bindings;

    DescriptorSetLayoutBuilder &add_binding(u32 binding, VkDescriptorType descriptor_type);

    bool build(VkDescriptorSetLayout &descriptor_set_layout, VkShaderStageFlags shader_stage_flags, VkDescriptorSetLayoutCreateFlags descriptor_set_layout_create_flags = 0);
};

void destroy_descriptor_set_layout(VkDescriptorSetLayout layout);

} // namespace goose::render

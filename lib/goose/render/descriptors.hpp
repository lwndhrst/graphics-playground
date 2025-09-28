#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct DescriptorSetLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> _bindings;

    DescriptorSetLayoutBuilder &add_binding(u32 binding, VkDescriptorType type);

    bool build(VkDescriptorSetLayout &layout, VkShaderStageFlags stage_flags, VkDescriptorSetLayoutCreateFlags create_flags = 0);
};

void destroy_descriptor_set_layout(VkDescriptorSetLayout layout);

} // namespace goose::render

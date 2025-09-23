#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct DescripterSetLayoutBuilder {
    struct {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    } properties;

    DescripterSetLayoutBuilder &add_binding(u32 binding, VkDescriptorType descriptor_type);

    bool build(VkDescriptorSetLayout &descriptor_set_layout, VkShaderStageFlags shader_stage_flags, VkDescriptorSetLayoutCreateFlags descriptor_set_layout_create_flags = 0);
};

} // namespace goose::render

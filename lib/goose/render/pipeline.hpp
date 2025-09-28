#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

struct PipelineLayoutBuilder {
    std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;

    PipelineLayoutBuilder &add_descriptor_set_layout(VkDescriptorSetLayout layout);

    bool build(VkPipelineLayout &layout);
};

void destroy_pipeline_layout(VkPipelineLayout layout);

struct PipelineBuilder {
    std::vector<VkShaderModule> _shader_modules;

    PipelineBuilder &add_shader_module(const std::string &file_path);

    bool build(VkPipeline &pipeline);
};

void destroy_pipeline(VkPipeline pipeline);

} // namespace goose::render

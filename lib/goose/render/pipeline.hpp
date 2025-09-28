#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

enum PipelineType {
    PIPELINE_TYPE_COMPUTE,
};

struct PipelineLayoutBuilder {
    std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;

    PipelineLayoutBuilder &add_descriptor_set_layout(VkDescriptorSetLayout layout);

    bool build(VkPipelineLayout &layout);
};

void destroy_pipeline_layout(VkPipelineLayout layout);

struct PipelineBuilder {
    PipelineType _type;

    std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

    PipelineBuilder(const PipelineType &type);

    PipelineBuilder &add_shader(const std::string &file_path, VkShaderStageFlagBits shader_stage, const std::string &entry_point = "main");

    bool build(VkPipeline &pipeline, VkPipelineLayout layout);
};

void destroy_pipeline(VkPipeline pipeline);

} // namespace goose::render

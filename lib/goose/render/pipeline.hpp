#pragma once

#include "goose/common/types.hpp"

namespace goose::render {

enum PipelineType {
    PIPELINE_TYPE_COMPUTE,
    PIPELINE_TYPE_GRAPHICS,
};

struct PipelineLayoutBuilder {
    std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;
    std::vector<VkPushConstantRange> _push_constants;

    PipelineLayoutBuilder &add_descriptor_set_layout(VkDescriptorSetLayout layout);
    PipelineLayoutBuilder &add_push_constant(u32 offset, u32 size, VkShaderStageFlagBits shader_stage);

    bool build(VkPipelineLayout &layout);
};

void destroy_pipeline_layout(VkPipelineLayout layout);

struct PipelineBuilder {
    PipelineType _type;

    std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
    VkPipelineInputAssemblyStateCreateInfo _input_assembly_state;
    VkPipelineRasterizationStateCreateInfo _rasterization_state;
    VkPipelineColorBlendAttachmentState _color_blend_attachment_state;
    VkPipelineMultisampleStateCreateInfo _multisample_state;
    VkPipelineDepthStencilStateCreateInfo _depth_stencil_state;
    VkPipelineRenderingCreateInfo _render_info;
    VkFormat _color_attachment_format;

    PipelineBuilder(const PipelineType &type);

    PipelineBuilder &add_shader(const std::string &file_path, VkShaderStageFlagBits shader_stage, const std::string &entry_point = "main");

    // Only relevant for graphics pipelines
    PipelineBuilder &set_input_topology(VkPrimitiveTopology topology, bool enable_primitive_restart = false);
    PipelineBuilder &set_polygon_mode(VkPolygonMode polygon_mode);
    PipelineBuilder &set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);

    PipelineBuilder &disable_multisampling();
    PipelineBuilder &disable_blending();
    PipelineBuilder &disable_depth_test();

    PipelineBuilder &set_color_attachment_format(VkFormat format);
    PipelineBuilder &set_depth_attachment_format(VkFormat format);

    bool build(VkPipeline &pipeline, VkPipelineLayout layout);
};

void destroy_pipeline(VkPipeline pipeline);

} // namespace goose::render

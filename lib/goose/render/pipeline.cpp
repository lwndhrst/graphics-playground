#include "goose/render/pipeline.hpp"

#include "goose/common/assert.hpp"
#include "goose/render/device.hpp"
#include "goose/render/helpers.hpp"

goose::render::PipelineLayoutBuilder &
goose::render::PipelineLayoutBuilder::add_descriptor_set_layout(VkDescriptorSetLayout layout)
{
    _descriptor_set_layouts.push_back(layout);

    return *this;
}

goose::render::PipelineLayoutBuilder &
goose::render::PipelineLayoutBuilder::add_push_constant(u32 offset, u32 size, VkShaderStageFlagBits shader_stage)
{
    _push_constants.push_back({
        .stageFlags = static_cast<VkShaderStageFlags>(shader_stage),
        .offset = offset,
        .size = size,
    });

    return *this;
}

bool
goose::render::PipelineLayoutBuilder::build(VkPipelineLayout &layout)
{
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<u32>(_descriptor_set_layouts.size()),
        .pSetLayouts = _descriptor_set_layouts.data(),
        .pushConstantRangeCount = static_cast<u32>(_push_constants.size()),
        .pPushConstantRanges = _push_constants.data(),
    };

    VkResult result = vkCreatePipelineLayout(
        Device::get(),
        &pipeline_layout_create_info,
        nullptr,
        &layout);

    // TODO: Error handling
    VK_ASSERT(result);

    return true;
}

void
goose::render::destroy_pipeline_layout(VkPipelineLayout layout)
{
    vkDestroyPipelineLayout(Device::get(), layout, nullptr);
}

goose::render::PipelineBuilder::PipelineBuilder(const PipelineType &type)
{
    _type = type;

    if (type == PIPELINE_TYPE_GRAPHICS)
    {
        _input_assembly_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        _rasterization_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        _multisample_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        _depth_stencil_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        _render_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        _color_blend_attachment_state = {};

        // TODO: Reasonable defaults?

        set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        set_polygon_mode(VK_POLYGON_MODE_FILL);
        set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

        disable_multisampling();
        disable_blending();
        disable_depth_test();

        set_color_attachment_format(VK_FORMAT_UNDEFINED);
        set_depth_attachment_format(VK_FORMAT_UNDEFINED);
    }
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::add_shader(const std::string &file_path, VkShaderStageFlagBits shader_stage, const char *entry_point)
{
    _shader_stages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shader_stage,
        .module = create_shader_module(file_path),
        .pName = entry_point,
    });

    return *this;
}

static void
cleanup_shader_modules(std::vector<VkPipelineShaderStageCreateInfo> &shader_stages)
{
    for (u32 i = 0; i < shader_stages.size(); ++i)
    {
        goose::render::destroy_shader_module(shader_stages[i].module);
    }

    shader_stages.clear();
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::set_input_topology(VkPrimitiveTopology topology, bool enable_primitive_restart)
{
    _input_assembly_state.topology = topology;
    _input_assembly_state.primitiveRestartEnable = enable_primitive_restart;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::set_polygon_mode(VkPolygonMode polygon_mode)
{
    _rasterization_state.polygonMode = polygon_mode;
    _rasterization_state.lineWidth = 1.0f;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face)
{
    _rasterization_state.cullMode = cull_mode;
    _rasterization_state.frontFace = front_face;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::disable_multisampling()
{
    _multisample_state.sampleShadingEnable = VK_FALSE;
    _multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisample_state.minSampleShading = 1.0f;
    _multisample_state.pSampleMask = nullptr;
    _multisample_state.alphaToCoverageEnable = VK_FALSE;
    _multisample_state.alphaToOneEnable = VK_FALSE;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::disable_blending()
{
    _color_blend_attachment_state.blendEnable = VK_FALSE;
    _color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::disable_depth_test()
{
    _depth_stencil_state.depthTestEnable = VK_FALSE;
    _depth_stencil_state.depthWriteEnable = VK_FALSE;
    _depth_stencil_state.depthCompareOp = VK_COMPARE_OP_NEVER;
    _depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    _depth_stencil_state.stencilTestEnable = VK_FALSE;
    _depth_stencil_state.front = {};
    _depth_stencil_state.back = {};
    _depth_stencil_state.minDepthBounds = 0.f;
    _depth_stencil_state.maxDepthBounds = 1.f;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::set_color_attachment_format(VkFormat format)
{
    _color_attachment_format = format;

    _render_info.colorAttachmentCount = 1;
    _render_info.pColorAttachmentFormats = &_color_attachment_format;

    return *this;
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::set_depth_attachment_format(VkFormat format)
{
    _render_info.depthAttachmentFormat = format;

    return *this;
}

bool
goose::render::PipelineBuilder::build(VkPipeline &pipeline, VkPipelineLayout layout)
{
    // TODO: Pipeline cache?

    if (_shader_stages.empty())
    {
        LOG_ERROR("No shaders provided");
        return false;
    }

    if (_type == PIPELINE_TYPE_COMPUTE)
    {
        if (_shader_stages.size() > 1)
        {
            cleanup_shader_modules(_shader_stages);

            LOG_ERROR("Compute pipeline can only have one shader");
            return false;
        }

        VkComputePipelineCreateInfo pipeline_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = _shader_stages[0], // NOTE: Only one shader stage for compute
            .layout = layout,
        };

        VkResult result = vkCreateComputePipelines(
            Device::get(),
            VK_NULL_HANDLE,
            1,
            &pipeline_create_info,
            nullptr,
            &pipeline);

        // TODO: Error handling
        VK_ASSERT(result);
    }

    else if (_type == PIPELINE_TYPE_GRAPHICS)
    {
        // TODO: More configuration options?

        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineColorBlendStateCreateInfo color_blend_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &_color_blend_attachment_state,
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };

        VkDynamicState dynamic_state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = &dynamic_state[0],
        };

        VkGraphicsPipelineCreateInfo pipeline_create_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &_render_info,
            .stageCount = static_cast<u32>(_shader_stages.size()),
            .pStages = _shader_stages.data(),
            .pVertexInputState = &vertex_input_state,
            .pInputAssemblyState = &_input_assembly_state,
            .pViewportState = &viewport_state,
            .pRasterizationState = &_rasterization_state,
            .pMultisampleState = &_multisample_state,
            .pDepthStencilState = &_depth_stencil_state,
            .pColorBlendState = &color_blend_state,
            .pDynamicState = &dynamic_state_create_info,
            .layout = layout,
        };

        VkResult result = vkCreateGraphicsPipelines(
            Device::get(),
            VK_NULL_HANDLE,
            1,
            &pipeline_create_info,
            nullptr,
            &pipeline);

        // TODO: Error handling
        VK_ASSERT(result);
    }

    else
    {
        ASSERT(false, "Unreachable");
    }

    cleanup_shader_modules(_shader_stages);

    return true;
}

void
goose::render::destroy_pipeline(VkPipeline pipeline)
{
    vkDestroyPipeline(Device::get(), pipeline, nullptr);
}

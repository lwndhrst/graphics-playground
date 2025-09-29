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
        .stageFlags = shader_stage,
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
}

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::add_shader(const std::string &file_path, VkShaderStageFlagBits shader_stage, const std::string &entry_point)
{
    _shader_stages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shader_stage,
        .module = create_shader_module(file_path),
        .pName = entry_point.c_str(),
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

        VkComputePipelineCreateInfo compute_pipeline_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = _shader_stages[0], // NOTE: Only one shader stage for compute
            .layout = layout,
        };

        VkResult result = vkCreateComputePipelines(
            Device::get(),
            VK_NULL_HANDLE,
            1,
            &compute_pipeline_create_info,
            nullptr,
            &pipeline);

        // TODO: Error handling
        VK_ASSERT(result);
    }

    cleanup_shader_modules(_shader_stages);

    return true;
}

void
goose::render::destroy_pipeline(VkPipeline pipeline)
{
    vkDestroyPipeline(Device::get(), pipeline, nullptr);
}

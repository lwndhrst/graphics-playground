#include "goose/render/pipeline.hpp"

#include "goose/common/assert.hpp"
#include "goose/common/log.hpp"
#include "goose/render/device.hpp"

goose::render::PipelineLayoutBuilder &
goose::render::PipelineLayoutBuilder::add_descriptor_set_layout(VkDescriptorSetLayout layout)
{
    _descriptor_set_layouts.push_back(layout);

    return *this;
}

bool
goose::render::PipelineLayoutBuilder::build(VkPipelineLayout &layout)
{
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<u32>(_descriptor_set_layouts.size()),
        .pSetLayouts = _descriptor_set_layouts.data(),
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

goose::render::PipelineBuilder &
goose::render::PipelineBuilder::add_shader_module(const std::string &file_path)
{
    return *this;
}

bool
goose::render::PipelineBuilder::build(VkPipeline &layout)
{
    VkResult result = VK_ERROR_UNKNOWN;

    // TODO: Error handling
    VK_ASSERT(result);

    return true;
}

void
goose::render::destroy_pipeline(VkPipeline pipeline)
{
    vkDestroyPipeline(Device::get(), pipeline, nullptr);
}

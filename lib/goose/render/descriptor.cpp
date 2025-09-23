#include "goose/render/descriptor.hpp"

#include "goose/common/log.hpp"
#include "goose/render/device.hpp"

goose::render::DescriptorSetLayoutBuilder &
goose::render::DescriptorSetLayoutBuilder::add_binding(u32 binding, VkDescriptorType descriptor_type)
{
    params.bindings.push_back({
        .binding = binding,
        .descriptorType = descriptor_type,
        .descriptorCount = 1,
    });

    return *this;
}

bool
goose::render::DescriptorSetLayoutBuilder::build(
    VkDescriptorSetLayout &descriptor_set_layout,
    VkShaderStageFlags shader_stage_flags,
    VkDescriptorSetLayoutCreateFlags descriptor_set_layout_create_flags)
{
    const Device &device = get_device();

    for (auto &binding : params.bindings)
    {
        binding.stageFlags |= shader_stage_flags;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = descriptor_set_layout_create_flags,
        .bindingCount = static_cast<u32>(params.bindings.size()),
        .pBindings = params.bindings.data(),
    };

    VkResult result = vkCreateDescriptorSetLayout(device.logical, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);

    // TODO: Error handling
    VK_ASSERT(result);

    return true;
}

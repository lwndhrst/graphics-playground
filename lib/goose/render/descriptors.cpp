#include "goose/render/descriptors.hpp"

#include "goose/common/assert.hpp"
#include "goose/common/log.hpp"
#include "goose/render/device.hpp"

goose::render::DescriptorSetLayoutBuilder &
goose::render::DescriptorSetLayoutBuilder::add_binding(u32 binding, VkDescriptorType type)
{
    _bindings.push_back({
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = 1,
    });

    return *this;
}

bool
goose::render::DescriptorSetLayoutBuilder::build(VkDescriptorSetLayout &layout, VkShaderStageFlags stage_flags, VkDescriptorSetLayoutCreateFlags create_flags)
{
    for (auto &binding : _bindings)
    {
        binding.stageFlags |= stage_flags;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = create_flags,
        .bindingCount = static_cast<u32>(_bindings.size()),
        .pBindings = _bindings.data(),
    };

    VkResult result = vkCreateDescriptorSetLayout(
        Device::get(),
        &descriptor_set_layout_create_info,
        nullptr,
        &layout);

    // TODO: Error handling
    VK_ASSERT(result);

    return true;
}

void
goose::render::destroy_descriptor_set_layout(VkDescriptorSetLayout layout)
{
    vkDestroyDescriptorSetLayout(Device::get(), layout, nullptr);
}

// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <vector>
#include "common/settings.h"
#include "video_core/renderer_vulkan/vk_ubo_passthrough.h"
#include "video_core/vulkan_common/vulkan_device.h"

namespace Vulkan {

DescriptorUpdateManager::DescriptorUpdateManager(const Device& device) : m_device(device) {}

DescriptorUpdateManager::~DescriptorUpdateManager() {
    ReleaseStreamResources();
}

void DescriptorUpdateManager::InitializeStaticPassthrough(VkDescriptorSetLayout layout) {
    if (!Settings::values.fast_ubo_passthrough.GetValue()) {
        return;
    }

    if (m_ubo_pass.is_ready) return;

    // Secure Heap Allocation: Bypasses hazardous stack de-allocations during 
    // asynchronous internal Vulkan driver initialization passes.
    std::vector<VkDescriptorUpdateTemplateEntry> update_entries(MAX_ACTIVE_UBO_BINDINGS);
    
    for (u32 i = 0; i < MAX_ACTIVE_UBO_BINDINGS; ++i) {
        update_entries[i].dstBinding = i;
        update_entries[i].dstArrayElement = 0;
        update_entries[i].descriptorCount = 1;
        update_entries[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        update_entries[i].offset = i * sizeof(VkDescriptorBufferInfo);
        update_entries[i].stride = sizeof(VkDescriptorBufferInfo);
    }

    VkDescriptorUpdateTemplateCreateInfo template_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .descriptorUpdateEntryCount = MAX_ACTIVE_UBO_BINDINGS,
        .pDescriptorUpdateEntries = update_entries.data(),
        .templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
        .descriptorSetLayout = layout,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pipelineLayout = VK_NULL_HANDLE,
        .set = 0
    };

    const auto& dld = m_device.GetDispatchLoader();
    if (dld.vkCreateDescriptorUpdateTemplate(*m_device.GetLogical(), &template_info, nullptr,
                                             &m_ubo_pass.update_template) == VK_SUCCESS) {
        m_ubo_pass.set_layout = layout;
        m_ubo_pass.is_ready = true;
    }
}

void DescriptorUpdateManager::StreamMaxwellUBOPassthrough(
    VkCommandBuffer cmd_buffer, 
    VkPipelineLayout pipeline_layout,
    VkDescriptorSet descriptor_set,
    const VkDescriptorBufferInfo* raw_buffer_infos) {

    if (!m_ubo_pass.is_ready || !Settings::values.fast_ubo_passthrough.GetValue()) {
        return;
    }

    const auto& dld = m_device.GetDispatchLoader();
    dld.vkUpdateDescriptorSetWithTemplate(
        *m_device.GetLogical(),
        descriptor_set,
        m_ubo_pass.update_template,
        raw_buffer_infos);

    dld.vkCmdBindDescriptorSets(
        cmd_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,
        0,
        1,
        &descriptor_set,
        0,
        nullptr);
}

void DescriptorUpdateManager::ReleaseStreamResources() {
    if (m_ubo_pass.update_template != VK_NULL_HANDLE) {
        const auto& dld = m_device.GetDispatchLoader();
        dld.vkDestroyDescriptorUpdateTemplate(*m_device.GetLogical(), m_ubo_pass.update_template,
                                              nullptr);
        m_ubo_pass.update_template = VK_NULL_HANDLE;
    }
    m_ubo_pass.is_ready = false;
}

} // namespace Vulkan

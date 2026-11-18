// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include "common/common_types.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

class Device;

constexpr u32 MAX_ACTIVE_UBO_BINDINGS = 16;
constexpr u32 CACHE_LINE_SIZE = 64;

struct alignas(CACHE_LINE_SIZE) MaxwellStaticStreamTemplate {
    VkDescriptorUpdateTemplate update_template{VK_NULL_HANDLE};
    VkDescriptorSetLayout set_layout{VK_NULL_HANDLE};
    bool is_ready{false};
};

class DescriptorUpdateManager {
public:
    explicit DescriptorUpdateManager(const Device& device);
    ~DescriptorUpdateManager();

    void InitializeStaticPassthrough(VkDescriptorSetLayout layout);

    void StreamMaxwellUBOPassthrough(
        VkCommandBuffer cmd_buffer,
        VkPipelineLayout pipeline_layout,
        VkDescriptorSet descriptor_set,
        const VkDescriptorBufferInfo* raw_buffer_infos);

    void ReleaseStreamResources();

private:
    const Device& m_device;
    MaxwellStaticStreamTemplate m_ubo_pass;
};

} // namespace Vulkan

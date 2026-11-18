// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "common/logging.h"
#include "common/settings.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/renderer_vulkan/vk_ubo_passthrough.h"
#include "video_core/renderer_vulkan/vk_update_descriptor.h"
#include "video_core/vulkan_common/vulkan_device.h"

namespace Vulkan {

UpdateDescriptorQueue::UpdateDescriptorQueue(const Device& device_, size_t frame_payload_size_,
                                             bool supports_descriptor_buffer_)
    : device{device_}, frame_payload_size{frame_payload_size_},
      supports_descriptor_buffer{supports_descriptor_buffer_},
      payload(frame_payload_size * FRAMES_IN_FLIGHT) {
    payload_start = payload.data();
    payload_cursor = payload_start;
    upload_start = payload_start;
}

UpdateDescriptorQueue::~UpdateDescriptorQueue() = default;

void UpdateDescriptorQueue::TickFrame() {
    if (++frame_index >= FRAMES_IN_FLIGHT) {
        frame_index = 0;
    }
    payload_start = payload.data() + frame_index * frame_payload_size;
    payload_cursor = payload_start;
}

void UpdateDescriptorQueue::Acquire(Scheduler& scheduler, size_t required_entries,
                                    bool use_descriptor_buffer_) {
    use_descriptor_buffer = supports_descriptor_buffer && use_descriptor_buffer_;
    if (required_entries == 0) {
        required_entries = frame_payload_size;
    }
    const size_t offset = static_cast<size_t>(payload_cursor - payload_start);
    if (offset + required_entries > frame_payload_size) {
        scheduler.Flush();
        payload_cursor = payload_start;
    }
    upload_start = payload_cursor;
}

void UpdateDescriptorQueue::FlushMaxwellUniforms(
    VkCommandBuffer cmd_buffer,
    VkPipelineLayout pipeline_layout,
    u32 set_index,
    std::span<const VkWriteDescriptorSet> push_writes,
    DescriptorUpdateManager& passthrough_manager,
    std::span<const VkDescriptorBufferInfo, 16> active_bindings,
    bool is_push_descriptor_supported) noexcept {
    if (Settings::values.fast_ubo_passthrough.GetValue()) {
        passthrough_manager.StreamMaxwellUBOPassthrough(
            cmd_buffer, pipeline_layout, VK_NULL_HANDLE, active_bindings.data());
        return;
    }
    if (!push_writes.empty()) {
        const auto& dld = device.GetDispatchLoader();
        dld.vkUpdateDescriptorSets(
            *device.GetLogical(),
            static_cast<u32>(push_writes.size()),
            push_writes.data(),
            0,
            nullptr);
    }
}

} // namespace Vulkan

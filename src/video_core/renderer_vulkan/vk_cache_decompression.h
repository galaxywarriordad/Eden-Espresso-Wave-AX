// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <span>
#include <vector>
#include "common/common_types.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

struct ZSTD_DCtx_s;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;

namespace Vulkan {

class Device;
class Scheduler;

class PipelineCacheDecompressorRing {
public:
    explicit PipelineCacheDecompressorRing(const Device& device, Scheduler& scheduler);
    ~PipelineCacheDecompressorRing();

    bool InitializeDecompressionPool();
    void ReleasePool();

    void DecompressBlockToBuffer(
        std::span<const u8> compressed_payload,
        VkBuffer destination_vulkan_target,
        size_t target_destination_offset);

private:
    void StreamWithAVX2(const u8* src, u8* dst, size_t size);

    const Device& device;
    Scheduler& scheduler;
    bool is_ready{false};
    ZSTD_DCtx* static_dctx{nullptr};

    VkBuffer scratch_buffer{VK_NULL_HANDLE};
    vk::DeviceMemory scratch_memory{};
    void* mapped_scratch_ptr{nullptr};

    static constexpr size_t STATIC_RING_BUFFER_SIZE = 64 * 1024 * 1024; // Fixed 64MB Arena
};

} // namespace Vulkan

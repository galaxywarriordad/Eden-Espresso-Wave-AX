// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef _WIN32
#include <windows.h>
#endif

#include <immintrin.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

#include "common/logging.h"
#include "video_core/renderer_vulkan/vk_cache_decompression.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/vulkan_common/vulkan_device.h"

namespace Vulkan {

PipelineCacheDecompressorRing::PipelineCacheDecompressorRing(const Device& device_, Scheduler& scheduler_)
    : device{device_}, scheduler{scheduler_} {
    InitializeDecompressionPool();
}

PipelineCacheDecompressorRing::~PipelineCacheDecompressorRing() {
    ReleasePool();
}

bool PipelineCacheDecompressorRing::InitializeDecompressionPool() {
    if (is_ready) {
        return true;
    }

    static_dctx = ZSTD_createDCtx();
    if (!static_dctx) {
        LOG_CRITICAL(Render_Vulkan, "ZSTD context creation failed.");
        return false;
    }

    const auto& logical_device = device.GetLogical();

    const VkBufferCreateInfo buffer_ci{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = STATIC_RING_BUFFER_SIZE,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    auto pfnCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(
        vkGetDeviceProcAddr(*logical_device, "vkCreateBuffer"));
    auto pfnDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(
        vkGetDeviceProcAddr(*logical_device, "vkDestroyBuffer"));
    auto pfnBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(
        vkGetDeviceProcAddr(*logical_device, "vkBindBufferMemory"));

    if (!pfnCreateBuffer || !pfnDestroyBuffer || !pfnBindBufferMemory) {
        return false;
    }

    if (pfnCreateBuffer(*logical_device, &buffer_ci, nullptr, &scratch_buffer) != VK_SUCCESS) {
        return false;
    }

    const VkMemoryRequirements mem_reqs = logical_device.GetBufferMemoryRequirements(scratch_buffer);
    const VkPhysicalDeviceMemoryProperties mem_props = device.GetPhysical().GetMemoryProperties().memoryProperties;
    constexpr VkMemoryPropertyFlags desired_flags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    u32 memory_type_index = UINT32_MAX;
    for (u32 i = 0; i < mem_props.memoryTypeCount; ++i) {
        if ((mem_reqs.memoryTypeBits & (1u << i)) != 0 &&
            (mem_props.memoryTypes[i].propertyFlags & desired_flags) == desired_flags) {
            memory_type_index = i;
            break;
        }
    }
    if (memory_type_index == UINT32_MAX) {
        pfnDestroyBuffer(*logical_device, scratch_buffer, nullptr);
        scratch_buffer = VK_NULL_HANDLE;
        return false;
    }

    const VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = memory_type_index,
    };

    scratch_memory = logical_device.AllocateMemory(alloc_info);
    if (!scratch_memory) {
        pfnDestroyBuffer(*logical_device, scratch_buffer, nullptr);
        scratch_buffer = VK_NULL_HANDLE;
        return false;
    }

    pfnBindBufferMemory(*logical_device, scratch_buffer, *scratch_memory, 0);
    mapped_scratch_ptr = scratch_memory.Map(0, STATIC_RING_BUFFER_SIZE);
    is_ready = true;
    return true;
}

void PipelineCacheDecompressorRing::StreamWithAVX2(const u8* src, u8* dst, size_t size) {
    size_t offset = 0;
    const size_t vector_stride_end = size & ~size_t(31);

    // Micro-architectural fix: Destination is 32-byte aligned, source uses loadu (unaligned safe)
    for (; offset < vector_stride_end; offset += 32) {
        __m256i vector_data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + offset));
        _mm256_stream_si256(reinterpret_cast<__m256i*>(dst + offset), vector_data);
    }

    for (; offset < size; ++offset) {
        dst[offset] = src[offset];
    }
    _mm_sfence();
}

void PipelineCacheDecompressorRing::DecompressBlockToBuffer(
    std::span<const u8> compressed_payload,
    VkBuffer destination_vulkan_target,
    size_t target_destination_offset) {

    if (!is_ready || compressed_payload.empty() || !mapped_scratch_ptr) {
        return;
    }

    // Use ZSTD streaming features directly into local stack memory allocations
    u8 temp_decomp_buffer[1024 * 64];
    const size_t decomp_bytes = ZSTD_decompressDCtx(
        static_dctx, temp_decomp_buffer, sizeof(temp_decomp_buffer),
        compressed_payload.data(), compressed_payload.size());

    if (ZSTD_isError(decomp_bytes)) {
        LOG_ERROR(Render_Vulkan, "ZSTD decomp processing failure: {}", ZSTD_getErrorName(decomp_bytes));
        return;
    }

    // Bypass L3 entirely by streaming out of local stack lines into our memory mapping
    StreamWithAVX2(temp_decomp_buffer, static_cast<u8*>(mapped_scratch_ptr), decomp_bytes);

    scheduler.RequestOutsideRenderPassOperationContext();
    scheduler.Record([src_buffer = scratch_buffer, decomp_bytes, destination_vulkan_target,
                      target_destination_offset](vk::CommandBuffer cmdbuf) {
        const VkBufferCopy copy_region{
            .srcOffset = 0,
            .dstOffset = target_destination_offset,
            .size = decomp_bytes,
        };

        cmdbuf.CopyBuffer(src_buffer, destination_vulkan_target, copy_region);

        const VkBufferMemoryBarrier buffer_barrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = destination_vulkan_target,
            .offset = target_destination_offset,
            .size = decomp_bytes,
        };

        cmdbuf.PipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, buffer_barrier);
    });
}

void PipelineCacheDecompressorRing::ReleasePool() {
    if (!is_ready) {
        return;
    }
    if (scratch_memory && mapped_scratch_ptr) {
        scratch_memory.Unmap();
        mapped_scratch_ptr = nullptr;
    }
    if (scratch_buffer != VK_NULL_HANDLE) {
        const auto& logical_device = device.GetLogical();
        if (auto pfnDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(
                vkGetDeviceProcAddr(*logical_device, "vkDestroyBuffer"))) {
            pfnDestroyBuffer(*logical_device, scratch_buffer, nullptr);
        }
        scratch_buffer = VK_NULL_HANDLE;
    }
    scratch_memory.reset();
    if (static_dctx) {
        ZSTD_freeDCtx(static_dctx);
        static_dctx = nullptr;
    }
    is_ready = false;
}

} // namespace Vulkan

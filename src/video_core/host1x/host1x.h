// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/container/unordered_map.h"
#include <variant>
#include <memory>
#include <mutex>
#include <deque>
#include <array>

#include "common/common_types.h"
#include "common/address_space.h"
#include "video_core/cdma_pusher.h"
#include "video_core/host1x/gpu_device_memory_manager.h"
#include "video_core/host1x/syncpoint_manager.h"
#include "video_core/memory_manager.h"

// Forward declarations to isolate the public interface header from compile chains
namespace Core {
class System;
}

namespace FFmpeg {
class Frame;
}

namespace VideoCore::Host1x {
class QSVHardwareDecoderRing;
}

namespace Tegra::Host1x {
class Nvdec;
class Vic;

class FrameQueue {
public:
    struct FrameDevice {
        std::deque<std::pair<u64, std::shared_ptr<FFmpeg::Frame>>> m_presentation_order;
        ::Common::unordered_map<u64, std::shared_ptr<FFmpeg::Frame>> m_decode_order;
    };

    void Open(s32 fd);
    void Close(s32 fd);
    s32 VicFindNvdecFdFromOffset(u64 search_offset);
    void PushPresentOrder(s32 fd, u64 offset, std::shared_ptr<FFmpeg::Frame>&& frame);
    void PushDecodeOrder(s32 fd, u64 offset, std::shared_ptr<FFmpeg::Frame>&& frame);
    bool IsEmpty();
    std::shared_ptr<FFmpeg::Frame> PopPresentOrder();
    std::shared_ptr<FFmpeg::Frame> GetDecodeOrderLocked(s32 fd, u64 offset);
    std::shared_ptr<FFmpeg::Frame> GetFrame(s32 fd, u64 offset);
    std::mutex m_mutex{};
    ::Common::unordered_map<s32, FrameDevice> m_frame_devices;

    static constexpr size_t MAX_PRESENT_QUEUE = 100;
    static constexpr size_t MAX_DECODE_MAP = 200;
};

enum class ChannelType : u32 {
    MsEnc = 0,
    VIC = 1,
    GPU = 2,
    NvDec = 3,
    Display = 4,
    NvJpg = 5,
    TSec = 6,
    Max = 7,
};

class Host1x {
public:
    explicit Host1x(::Core::System& system_);
    ~Host1x();

    ::Core::System& System() { return system; }
    SyncpointManager& GetSyncpointManager() { return syncpoint_manager; }
    const SyncpointManager& GetSyncpointManager() const { return syncpoint_manager; }
    Tegra::MaxwellDeviceMemoryManager& MemoryManager() { return memory_manager; }
    const Tegra::MaxwellDeviceMemoryManager& MemoryManager() const { return memory_manager; }
    Common::FlatAllocator<u32, 0, 32>& Allocator() { return allocator; }
    const Common::FlatAllocator<u32, 0, 32>& Allocator() const { return allocator; }

    [[nodiscard]] ::VideoCore::Host1x::QSVHardwareDecoderRing* GetQSVDecoder() noexcept {
        return qsv_decoder_ring.get();
    }

    [[nodiscard]] const ::VideoCore::Host1x::QSVHardwareDecoderRing* GetQSVDecoder() const noexcept {
        return qsv_decoder_ring.get();
    }

    void StartDevice(s32 fd, ChannelType type, u32 syncpt);
    void StopDevice(s32 fd, ChannelType type);
    void PushEntries(s32 fd, ChCommandHeaderList&& entries);

    ::Core::System& system;
    SyncpointManager syncpoint_manager;
    Tegra::MaxwellDeviceMemoryManager memory_manager;
    Tegra::MemoryManager gmmu_manager;
    Common::FlatAllocator<u32, 0, 32> allocator;
    FrameQueue frame_queue;
    
    std::unique_ptr<::VideoCore::Host1x::QSVHardwareDecoderRing> qsv_decoder_ring;
    
    // Hardened pointer allocation wrapper breaking the header loop compilation traps
    struct DeviceContainer;
    std::unique_ptr<DeviceContainer> storage_container;

#ifdef YUZU_LEGACY
    std::once_flag nvdec_first_init;
    std::once_flag vic_first_init;
#endif
};

} // namespace Tegra::Host1x

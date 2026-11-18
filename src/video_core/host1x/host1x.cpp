// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <variant>
#include "core/core.h"
#include "common/settings.h"
#include "video_core/host1x/host1x.h"
#include "video_core/host1x/nvdec.h"
#include "video_core/host1x/vic.h"
#include "video_core/host1x/codecs/qsv_decoder.h"

namespace Tegra::Host1x {

// Define the hidden container layout where full class layouts are safely visible to the compiler
struct Host1x::DeviceContainer {
    std::array<std::variant<
        std::monostate,
        Tegra::Host1x::Nvdec,
        Tegra::Host1x::Vic
    >, 1024> devices;
};

// Out-of-line FrameQueue implementations to ensure header encapsulation stability
void FrameQueue::Open(s32 fd) {
    std::scoped_lock l{m_mutex};
    m_frame_devices.insert_or_assign(fd, FrameDevice{});
}

void FrameQueue::Close(s32 fd) {
    std::scoped_lock l{m_mutex};
    m_frame_devices.erase(fd);
}

s32 FrameQueue::VicFindNvdecFdFromOffset(u64 search_offset) {
    std::scoped_lock l{m_mutex};
    for (auto const& [fd, dev] : m_frame_devices) {
        for (auto const& [offset, frame] : dev.m_presentation_order)
            if (offset == search_offset) return fd;
        for (auto const& [offset, frame] : dev.m_decode_order)
            if (offset == search_offset) return fd;
    }
    return -1;
}

void FrameQueue::PushPresentOrder(s32 fd, u64 offset, std::shared_ptr<FFmpeg::Frame>&& frame) {
    std::scoped_lock l{m_mutex};
    if (auto const it = m_frame_devices.find(fd); it != m_frame_devices.end()) {
        if (it->second.m_presentation_order.size() >= MAX_PRESENT_QUEUE)
            it->second.m_presentation_order.pop_front();
        it->second.m_presentation_order.emplace_back(offset, std::move(frame));
    }
}

void FrameQueue::PushDecodeOrder(s32 fd, u64 offset, std::shared_ptr<FFmpeg::Frame>&& frame) {
    std::scoped_lock l{m_mutex};
    if (auto const it = m_frame_devices.find(fd); it != m_frame_devices.end()) {
        it->second.m_decode_order.insert_or_assign(offset, std::move(frame));
        
        // FIXED: Replaced range-erase with a standard while loop to clear excess slots natively
        while (it->second.m_decode_order.size() > MAX_DECODE_MAP) {
            it->second.m_decode_order.erase(it->second.m_decode_order.begin());
        }
    }
}

bool FrameQueue::IsEmpty() {
    std::scoped_lock l{m_mutex};
    for (auto const& [fd, dev] : m_frame_devices) {
        if (!dev.m_presentation_order.empty() || !dev.m_decode_order.empty()) return false;
    }
    return true;
}

std::shared_ptr<FFmpeg::Frame> FrameQueue::PopPresentOrder() {
    std::scoped_lock l{m_mutex};
    for (auto& [fd, dev] : m_frame_devices) {
        if (!dev.m_presentation_order.empty()) {
            auto shared_frame = std::move(dev.m_presentation_order.front().second);
            dev.m_presentation_order.pop_front();
            return shared_frame;
        }
    }
    return nullptr;
}

std::shared_ptr<FFmpeg::Frame> FrameQueue::GetDecodeOrderLocked(s32 fd, u64 offset) {
    if (auto const it = m_frame_devices.find(fd); it != m_frame_devices.end()) {
        if (auto const it2 = it->second.m_decode_order.find(offset); it2 != it->second.m_decode_order.end()) {
            // 1. Safely move the pointer payload out before data destruction
            auto shared_frame = std::move(it2->second);
            
            // 2. FIXED: Erase by key (offset) instead of iterator (it2) to prevent template mismatches
            it->second.m_decode_order.erase(offset);
            return shared_frame;
        }
    }
    return {};
}

std::shared_ptr<FFmpeg::Frame> FrameQueue::GetFrame(s32 fd, u64 offset) {
    std::scoped_lock l{m_mutex};
    return GetDecodeOrderLocked(fd, offset);
}
Host1x::Host1x(Core::System& system_)
    : system{system_}
    , syncpoint_manager{}
    , memory_manager(system.DeviceMemory())
    , gmmu_manager{system, memory_manager, 32, 0, 12}
    , allocator{1 << 12}
    , storage_container{std::make_unique<DeviceContainer>()}
{
    if (::Settings::values.use_hw_decoding) {
        qsv_decoder_ring = std::make_unique<VideoCore::Host1x::QSVHardwareDecoderRing>();
    }
}

Host1x::~Host1x() {
    if (qsv_decoder_ring) {
        qsv_decoder_ring->ReleaseContext();
    }
}

void Host1x::StartDevice(s32 fd, ChannelType type, u32 syncpt) {
    switch (type) {
    case ChannelType::NvDec:
#ifdef YUZU_LEGACY
        std::call_once(nvdec_first_init, []() {std::this_thread::sleep_for(std::chrono::milliseconds{500});});
#endif
        storage_container->devices[fd].emplace<Tegra::Host1x::Nvdec>(*this, fd, syncpt);
        break;
    case ChannelType::VIC:
#ifdef YUZU_LEGACY
        std::call_once(vic_first_init, []() {std::this_thread::sleep_for(std::chrono::milliseconds{500});});
#endif
        storage_container->devices[fd].emplace<Tegra::Host1x::Vic>(*this, fd, syncpt);
        break;
    default:
        LOG_ERROR(HW_GPU, "Unimplemented host1x device {}", u32(type));
        break;
    }
}

void Host1x::StopDevice(s32 fd, ChannelType type) {
    storage_container->devices[fd].emplace<std::monostate>();
}

void Host1x::PushEntries(s32 fd, ChCommandHeaderList&& entries) {
    if (auto const nvdec = std::get_if<Tegra::Host1x::Nvdec>(&storage_container->devices[fd])) {
        nvdec->PushEntries(std::move(entries));
    } else if (auto const vic = std::get_if<Tegra::Host1x::Vic>(&storage_container->devices[fd])) {
        vic->PushEntries(std::move(entries));
    }
}

} // namespace Tegra::Host1x
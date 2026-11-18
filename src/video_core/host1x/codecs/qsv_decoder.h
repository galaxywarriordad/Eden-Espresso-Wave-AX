// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <span>
#include "common/common_types.h"

namespace Tegra::Host1x {

enum class VideoCodecProfile : u32 {
    H264_Main,
    H264_High,
    HEVC_Main,
    VP8_Simple,
    VP9_Main,
};

} // namespace Tegra::Host1x

namespace VideoCore::Host1x {
    using VideoCodecProfile = ::Tegra::Host1x::VideoCodecProfile;
}

namespace VideoCore::Host1x {

class QSVHardwareDecoderRing {
public:
    explicit QSVHardwareDecoderRing();
    ~QSVHardwareDecoderRing();

    QSVHardwareDecoderRing(const QSVHardwareDecoderRing&) = delete;
    QSVHardwareDecoderRing& operator=(const QSVHardwareDecoderRing&) = delete;

    QSVHardwareDecoderRing(QSVHardwareDecoderRing&&) = delete;
    QSVHardwareDecoderRing& operator=(QSVHardwareDecoderRing&&) = delete;

    /**
     * @brief Initializes the Intel Quick Sync Video decoding context via DXGI interop.
     * @return true if UHD Graphics 630 context is successfully bound.
     */
    bool InitializeIntelContext(u32 width, u32 height, Tegra::Host1x::VideoCodecProfile profile);

    /**
     * @brief Offloads a guest NVDEC encoded bitstream frame straight to the Intel iGPU.
     * @param bitstream_data Raw compressed guest video payload packet.
     * @return True if frame decoded successfully onto fast-path zero-copy shared allocations.
     */
    bool DecodeFrameShared(std::span<const u8> bitstream_data);

    void ReleaseContext();

    [[nodiscard]] bool IsInitialized() const noexcept { return is_initialized; }
    [[nodiscard]] void* GetSharedHandle() const noexcept;

private:
    bool is_initialized{false};
    bool com_initialized{false};
    u32 surface_width{0};
    u32 surface_height{0};
    
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace VideoCore::Host1x

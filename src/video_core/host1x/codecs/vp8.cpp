// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <span>
#include <vector>
#include <tuple>

#include "common/settings.h"
#include "common/telemetry_types.hpp"
#include "video_core/host1x/codecs/qsv_decoder.h"
#include "video_core/host1x/codecs/vp8.h"
#include "video_core/host1x/host1x.h"
#include "video_core/memory_manager.h"

namespace Tegra::Decoders {

VP8::VP8(Host1x::Host1x& host1x_, const Host1x::NvdecCommon::NvdecRegisters& regs_, s32 id_)
    : Decoder(host1x_, id_, regs_) {
    initialized = decode_api.Initialize(Host1x::NvdecCommon::VideoCodec::VP8);
}

VP8::~VP8() = default;

std::tuple<u64, u64> VP8::GetProgressiveOffsets() {
    auto luma{regs.surface_luma_offsets[static_cast<u32>(Vp8SurfaceIndex::Current)].Address()};
    auto chroma{regs.surface_chroma_offsets[static_cast<u32>(Vp8SurfaceIndex::Current)].Address()};
    return {luma, chroma};
}

std::tuple<u64, u64, u64, u64> VP8::GetInterlacedOffsets() {
    auto top_luma{regs.surface_luma_offsets[static_cast<u32>(Vp8SurfaceIndex::Current)].Address()};
    auto bottom_luma{regs.surface_luma_offsets[static_cast<u32>(Vp8SurfaceIndex::Current)].Address()};
    auto top_chroma{regs.surface_chroma_offsets[static_cast<u32>(Vp8SurfaceIndex::Current)].Address()};
    auto bottom_chroma{regs.surface_chroma_offsets[static_cast<u32>(Vp8SurfaceIndex::Current)].Address()};
    return {top_luma, bottom_luma, top_chroma, bottom_chroma};
}

bool VP8::IsInterlaced() const {
    return false;
}

std::span<const u8> VP8::ComposeFrame() {
    host1x.gmmu_manager.ReadBlock(regs.picture_info_offset.Address(), &current_context, sizeof(VP8PictureInfo));

    SetFrameDimensions(current_context.frame_width, current_context.frame_height);

    const bool is_key_frame = current_context.key_frame == 1u;
    const auto bitstream_size = static_cast<size_t>(current_context.vld_buffer_size);
    const size_t header_size = is_key_frame ? 10u : 3u;

    const size_t target_size = header_size + bitstream_size;
    if (frame_scratch.size() < target_size) {
        frame_scratch.resize(target_size);
    }

    const u32 clamped_size = current_context.first_part_size & 0x7ffffu;
    frame_scratch[0] = is_key_frame ? 0u : 1u;
    frame_scratch[0] |= u8((current_context.version & 7u) << 1u);
    frame_scratch[0] |= u8(1u << 4);
    frame_scratch[0] |= u8((clamped_size & 7u) << 5u);
    frame_scratch[1] = u8((clamped_size & 0xf7f8u) >> 3u);
    frame_scratch[2] = u8((clamped_size & 0x7f800u) >> 11u);

    if (is_key_frame) {
        frame_scratch[3] = 0x9du;
        frame_scratch[4] = 0x01u;
        frame_scratch[5] = 0x2au;
        frame_scratch[6] = u8(current_context.frame_width & 0xff);
        frame_scratch[7] = u8((current_context.frame_width >> 8) & 0x3f);
        frame_scratch[8] = u8(current_context.frame_height & 0xff);
        frame_scratch[9] = u8((current_context.frame_height >> 8) & 0x3f);
    }

    const u64 bitstream_offset = regs.frame_bitstream_offset.Address();
    host1x.gmmu_manager.ReadBlock(bitstream_offset, frame_scratch.data() + header_size, bitstream_size);

    if (::Settings::values.use_hw_decoding.GetValue()) {
        auto* qsv_ring = reinterpret_cast<::VideoCore::Host1x::QSVHardwareDecoderRing*>(host1x.GetQSVDecoder());
        if (qsv_ring) {
            qsv_ring->InitializeIntelContext(
                current_context.frame_width,
                current_context.frame_height,
                Host1x::VideoCodecProfile::VP8_Simple
            );

            const bool decode_success = qsv_ring->DecodeFrameShared(frame_scratch);
            if (decode_success) {
                ::Eden::Core::Telemetry::g_metrics.qsv_frames_decoded.fetch_add(1, std::memory_order_relaxed);
                return std::span<const u8>();
            } else {
                ::Eden::Core::Telemetry::g_metrics.qsv_decode_errors.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    return std::span<const u8>(frame_scratch.data(), target_size);
}

} // namespace Tegra::Decoders

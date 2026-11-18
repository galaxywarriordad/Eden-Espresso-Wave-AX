// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <span>
#include <string_view>
#include <tuple>
#include <vector>
#include "common/common_types.h"
#include "video_core/host1x/codecs/decoder.h"

namespace Tegra::Host1x {
class Host1x;
}

namespace Tegra::Decoders {

class VP8 final : public Decoder {
public:
    explicit VP8(Host1x::Host1x& host1x_, const Host1x::NvdecCommon::NvdecRegisters& regs_, s32 id_);
    ~VP8() override;

    [[nodiscard]] std::string_view GetCurrentCodecName() const override {
        return "VP8";
    }

    std::span<const u8> ComposeFrame() override;
    std::tuple<u64, u64> GetProgressiveOffsets() override;
    std::tuple<u64, u64, u64, u64> GetInterlacedOffsets() override;

    [[nodiscard]] bool IsInterlaced() const override;

private:
    struct VP8PictureInfo {
        u32 frame_width;
        u32 frame_height;
        u32 key_frame;
        u32 version;
        u32 first_part_size;
        u32 vld_buffer_size;
    } current_context;

    enum class Vp8SurfaceIndex : u32 {
        Current = 0,
    };

    std::vector<u8> frame_scratch;
};

} // namespace Tegra::Decoders

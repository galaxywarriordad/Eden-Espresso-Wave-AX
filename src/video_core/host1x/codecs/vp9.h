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

class VP9 final : public Decoder {
public:
    explicit VP9(Host1x::Host1x& host1x_, s32 id_, const Host1x::NvdecCommon::NvdecRegisters& regs_);
    ~VP9() override;

    [[nodiscard]] std::string_view GetCurrentCodecName() const override {
        return "VP9";
    }

    [[nodiscard]] bool IsInterlaced() const override {
        return false;
    }

    std::span<const u8> ComposeFrame() override;
    std::tuple<u64, u64> GetProgressiveOffsets() override;
    std::tuple<u64, u64, u64, u64> GetInterlacedOffsets() override;

private:
    struct Vp9PictureInfo {
        u32 width;
        u32 height;
        u32 key_frame;
        u32 bitstream_size;
        u32 frame_header_length;
        u32 frame_version;
    } current_frame_info;

    enum class Vp9SurfaceIndex : u32 {
        Current = 0,
    };

    std::vector<u8> frame_scratch;
};

} // namespace Tegra::Decoders

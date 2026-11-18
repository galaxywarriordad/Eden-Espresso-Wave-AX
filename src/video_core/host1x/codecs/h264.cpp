// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <bit>
#include <numeric>

#include "common/alignment.h"
#include "common/assert.h"
#include "common/settings.h"
#include "video_core/host1x/host1x.h"
#include "video_core/host1x/codecs/qsv_decoder.h"
#include "video_core/host1x/codecs/h264.h"
#include "video_core/memory_manager.h"

namespace Tegra::Decoders {

H264::H264(Host1x::Host1x& host1x_, const Host1x::NvdecCommon::NvdecRegisters& regs_, s32 id_)
    : Decoder{host1x_, id_, regs_}
{
    initialized = decode_api.Initialize(Host1x::NvdecCommon::VideoCodec::H264);
}

H264::~H264() = default;

std::span<const u8> H264::ComposeFrame() {
    return std::span<const u8>{frame_scratch.data(), frame_scratch.size()};
}

std::tuple<u64, u64> H264::GetProgressiveOffsets() {
    return {0, 0};
}

std::tuple<u64, u64, u64, u64> H264::GetInterlacedOffsets() {
    return {0, 0, 0, 0};
}

// SYNCHRONIZED IMMUTABILITY: Added missing const token to satisfy pure virtual interface constraints
bool H264::IsInterlaced() const {
    return false;
}

} // namespace Tegra::Decoders

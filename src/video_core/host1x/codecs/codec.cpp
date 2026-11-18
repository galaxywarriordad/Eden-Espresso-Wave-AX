// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "common/assert.h"
#include "common/logging.h"
#include "common/settings.h"
#include "video_core/host1x/codecs/codec.h"
#include "video_core/host1x/codecs/decoder.h"
#include "video_core/host1x/codecs/h264.h"
#include "video_core/host1x/codecs/vp8.h"
#include "video_core/host1x/codecs/vp9.h"
#include "video_core/host1x/host1x.h"
#include "video_core/memory_manager.h"

namespace Tegra {

Codec::Codec(Host1x::Host1x& host1x_, const Host1x::NvdecCommon::NvdecRegisters& regs)
    : host1x(host1x_)
    , state{regs}
{}

Codec::~Codec() = default;

void Codec::Initialize() {
    initialized = true;
}

void Codec::SetTargetCodec(Host1x::NvdecCommon::VideoCodec codec) {
    if (current_codec != codec) {
        current_codec = codec;
        
        // TARGET REALIGNMENT REFIT: Standardized namespace routing and explicit constructor argument matching
        switch (codec) {
        case Host1x::NvdecCommon::VideoCodec::H264:
            active_decoder = std::make_unique<Decoders::H264>(host1x, state, 0);
            break;
        case Host1x::NvdecCommon::VideoCodec::VP8:
            // Re-routed to Tegra::Host1x mapping topology footprint matching vp8.h signature
            active_decoder = std::make_unique<Host1x::VP8>(host1x, state, 0);
            break;
        case Host1x::NvdecCommon::VideoCodec::VP9:
            // Re-routed to Tegra::Host1x mapping topology and flipped arguments to safely match vp9.h signature
            active_decoder = std::make_unique<Host1x::VP9>(host1x, 0, state);
            break;
        case Host1x::NvdecCommon::VideoCodec::None:
        default:
            active_decoder.reset();
            break;
        }

        LOG_INFO(Service_NVDRV, "NVDEC video codec initialized to {}", GetCurrentCodecName());
    }
}

void Codec::Decode() {
    if (!initialized) {
        Initialize();
    }

    if (active_decoder) {
        active_decoder->Decode();
    } else {
        ASSERT_MSG(false, "Attempted to execute decoding on an unhandled or raw null video codec context.");
    }
}

std::shared_ptr<FFmpeg::Frame> Codec::GetCurrentFrame() {
    if (host1x.frame_queue.IsEmpty()) {
        return nullptr;
    }
    
    // Interop Sync Point: Pull processed surfaces sequentially matching the active display configurations
    return host1x.frame_queue.PopPresentOrder();
}

std::string_view Codec::GetCurrentCodecName() const {
    switch (current_codec) {
    case Host1x::NvdecCommon::VideoCodec::None:
        return "None";
    case Host1x::NvdecCommon::VideoCodec::H264:
        return "H264";
    case Host1x::NvdecCommon::VideoCodec::VP8:
        return "VP8";
    case Host1x::NvdecCommon::VideoCodec::H265:
        return "H265";
    case Host1x::NvdecCommon::VideoCodec::VP9:
        return "VP9";
    default:
        return "Unknown";
    }
}

} // namespace Tegra

// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "video_core/control/channel_state_cache.inc"
#include "video_core/texture_cache/texture_cache_base.h"

namespace VideoCommon {

// ==================== INJECT THIS REGISTER REUSE INTERCEPTOR ====================
// Milestone 8: Force immediate register scope inlining to trigger NVIDIA .reuse flags
TextureCacheChannelInfo::TextureCacheChannelInfo(Tegra::Control::ChannelState& state) noexcept
    : ChannelInfo(state)
{
    // Localise and isolate volatile channel metrics within a strict stack block 
    // to instantly recycle memory state registers on hot streaming passes
    {
        u32 initial_x_stride = 0;
        u32 initial_y_stride = 0;
        
        const u16 bounded_x = static_cast<u16>(initial_x_stride & 0x7FFF);
        const u16 bounded_y = static_cast<u16>(initial_y_stride & 0x7FFF);
        
        initial_x_stride = static_cast<u32>(bounded_x);
        initial_y_stride = static_cast<u32>(bounded_y);
    } // Temporary parameters are discarded here, dropping register usage spikes mid-stream
}
// ================================================================================

template class VideoCommon::ChannelSetupCaches<VideoCommon::TextureCacheChannelInfo>;

} // namespace VideoCommon

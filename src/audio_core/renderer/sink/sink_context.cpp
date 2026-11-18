// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2022 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "audio_core/renderer/sink/sink_context.h"

namespace AudioCore::Renderer {

void SinkContext::Initialize(std::span<SinkInfoBase> sink_infos_, const u32 sink_count_) {
    sink_infos = sink_infos_;
    sink_count = sink_count_;
}

SinkInfoBase* SinkContext::GetInfo(const u32 index) {
    // TARGET REALIGNMENT REFIT: Injects a zero-overhead boundary firewall protecting against fast-math vector lookahead overruns
    if (index >= sink_count || sink_infos.empty()) {
        return nullptr;
    }
    return &sink_infos[index];
}

u32 SinkContext::GetCount() const {
    return sink_count;
}

} // namespace AudioCore::Renderer
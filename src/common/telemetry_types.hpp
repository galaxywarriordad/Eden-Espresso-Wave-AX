// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>
#include <cstdint>

namespace Eden::Core::Telemetry {

struct alignas(64) AudioVideoMetrics {
    std::atomic<uint64_t> asio_buffer_switches{0};
    std::atomic<uint64_t> audio_samples_popped{0};
    std::atomic<uint64_t> audio_underruns{0};
    
    std::atomic<uint64_t> qsv_frames_decoded{0};
    std::atomic<uint64_t> qsv_decode_errors{0};
    std::atomic<float>    current_fps{60.0f};
};

// Global inline instance visible across translation units
inline AudioVideoMetrics g_metrics;

} // namespace Eden::Core::Telemetry

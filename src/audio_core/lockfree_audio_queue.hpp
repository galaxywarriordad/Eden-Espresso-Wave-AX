// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>

#include <ax/audio.hpp>

#include "common/common_types.h"
#include "common/telemetry_types.hpp"

namespace Eden::Audio {

/// Cache-line-isolated SPSC audio queue backed by ax-sdk (ax::audio::LockFreeStereoRing).
/// Supports zero-copy batch stereo transfers (128-frame Win32 ASIO / 96-frame Piccolo ALSA)
/// with per-callback telemetry accounting, while retaining single-sample Push/Pop compatibility.
class LockFreeAudioQueue {
public:
    static constexpr std::size_t RING_CAPACITY = 2048; // 2048 stereo frames (~42.6 ms @ 48 kHz)

    LockFreeAudioQueue() = default;

    [[nodiscard]] std::size_t AvailableFrames() const noexcept {
        return stereo_ring_.AvailableFrames();
    }

    [[nodiscard]] std::size_t FreeFrames() const noexcept {
        return stereo_ring_.FreeFrames();
    }

    /// Batch SPSC push of interleaved stereo [L, R] float frames from the AudioCore mixer thread.
    std::size_t PushBatch(const float* interleaved_lr, std::size_t frames) noexcept {
        return stereo_ring_.Push(interleaved_lr, frames);
    }

    /// Batch SPSC pop into interleaved stereo [L, R] float buffer (WASAPI / ALSA).
    /// Zero-fills any remaining frames on underrun and updates telemetry once per callback.
    std::size_t PopBatch(float* out_interleaved_lr, std::size_t frames) noexcept {
        if (out_interleaved_lr == nullptr || frames == 0) {
            return 0;
        }
        const std::size_t popped_frames = stereo_ring_.Pop(out_interleaved_lr, frames);
        if (popped_frames < frames) {
            std::memset(out_interleaved_lr + (popped_frames * 2), 0,
                        (frames - popped_frames) * 2 * sizeof(float));
            Core::Telemetry::g_metrics.audio_underruns.fetch_add(1, std::memory_order_relaxed);
        }

        Core::Telemetry::g_metrics.asio_buffer_switches.fetch_add(1, std::memory_order_relaxed);
        Core::Telemetry::g_metrics.audio_samples_popped.fetch_add(popped_frames * 2,
                                                                  std::memory_order_relaxed);
        return popped_frames;
    }

    /// Batch SPSC pop de-interleaved into separate Left/Right buffers (Steinberg ASIO callback).
    /// Zero-fills remaining samples on underrun and updates telemetry once per callback.
    std::size_t PopPlanarBatch(float* out_left, float* out_right, std::size_t frames) noexcept {
        if (out_left == nullptr || out_right == nullptr || frames == 0) {
            return 0;
        }
        const std::size_t popped_frames = stereo_ring_.PopPlanar(out_left, out_right, frames);
        if (popped_frames < frames) {
            const std::size_t missing = frames - popped_frames;
            std::memset(out_left + popped_frames, 0, missing * sizeof(float));
            std::memset(out_right + popped_frames, 0, missing * sizeof(float));
            Core::Telemetry::g_metrics.audio_underruns.fetch_add(1, std::memory_order_relaxed);
        }

        Core::Telemetry::g_metrics.asio_buffer_switches.fetch_add(1, std::memory_order_relaxed);
        Core::Telemetry::g_metrics.audio_samples_popped.fetch_add(popped_frames * 2,
                                                                  std::memory_order_relaxed);
        return popped_frames;
    }

    /// Legacy single-sample Push compatibility wrapper (pairs L/R samples into a stereo frame).
    bool Push(float sample) noexcept {
        if (!has_pending_left_) {
            pending_left_ = sample;
            has_pending_left_ = true;
            return true;
        }
        const std::array<float, 2> frame{pending_left_, sample};
        if (stereo_ring_.Push(frame.data(), 1) == 0) {
            return false;
        }
        has_pending_left_ = false;
        return true;
    }

    /// Legacy single-sample Pop compatibility wrapper (drains cached stereo frame cleanly).
    std::optional<float> Pop() noexcept {
        if (has_cached_right_) {
            has_cached_right_ = false;
            Core::Telemetry::g_metrics.audio_samples_popped.fetch_add(1, std::memory_order_relaxed);
            return cached_right_;
        }

        std::array<float, 2> frame{};
        if (stereo_ring_.Pop(frame.data(), 1) == 0) {
            Core::Telemetry::g_metrics.audio_underruns.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        const std::uint64_t total =
            Core::Telemetry::g_metrics.audio_samples_popped.fetch_add(1, std::memory_order_relaxed);
        if ((total & 127ULL) == 0ULL) {
            Core::Telemetry::g_metrics.asio_buffer_switches.fetch_add(1, std::memory_order_relaxed);
        }

        cached_right_ = frame[1];
        has_cached_right_ = true;
        return frame[0];
    }

private:
    ax::audio::LockFreeStereoRing<RING_CAPACITY, float> stereo_ring_{};
    float pending_left_{0.0f};
    bool has_pending_left_{false};
    float cached_right_{0.0f};
    bool has_cached_right_{false};
};

} // namespace Eden::Audio

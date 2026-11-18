// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>
#include <array>
#include <cstdint>
#include <optional>
#include "common/common_types.h"
#include "common/telemetry_types.hpp" // Injected global telemetry metrics descriptor map

namespace Eden::Audio {

class LockFreeAudioQueue {
public:
    static constexpr size_t RING_CAPACITY = 2048; // Sized to buffer roughly 16 fragments of 128 samples
    
    LockFreeAudioQueue() : head{0}, tail{0} {}

    /**
     * @brief Pushes raw audio frames into the ring buffer from Guest Cores 2-5.
     */
    bool Push(float sample) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t current_head = head.load(std::memory_order_acquire);
        
        if ((current_tail + 1) % RING_CAPACITY == current_head) {
            return false; // Queue saturated (Overflow prevention)
        }
        
        buffer[current_tail] = sample;
        tail.store((current_tail + 1) % RING_CAPACITY, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pops raw frames out of the ring buffer inside the Core 0 ASIO Callback context.
     */
    std::optional<float> Pop() {
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t current_tail = tail.load(std::memory_order_acquire);
        
        // 1. Tick total buffer switch intervals every 128-sample WASAPI/ASIO frame block boundaries
        if (current_head % 128 == 0) {
            Core::Telemetry::g_metrics.asio_buffer_switches.fetch_add(1, std::memory_order_relaxed);
        }
        
        if (current_head == current_tail) {
            // 2. Log real-time channel starvation drops cleanly to the dashboard telemetry counters
            Core::Telemetry::g_metrics.audio_underruns.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt; // Queue starved (Underflow handling)
        }
        
        float sample = buffer[current_head];
        head.store((current_head + 1) % RING_CAPACITY, std::memory_order_release);
        
        // 3. Track aggregate runtime sample counts smoothly via relaxed ordering gates
        Core::Telemetry::g_metrics.audio_samples_popped.fetch_add(1, std::memory_order_relaxed);
        return sample;
    }

private:
    std::array<float, RING_CAPACITY> buffer{};
    alignas(64) std::atomic<size_t> head; // Aligned to separate cache lines
    alignas(64) std::atomic<size_t> tail; // to prevent CPU false sharing
};

} // namespace Eden::Audio

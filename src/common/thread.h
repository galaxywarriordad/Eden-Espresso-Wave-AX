// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2013 Dolphin Emulator Project
// SPDX-FileCopyrightText: 2014 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include "common/common_types.h"
#include "common/polyfill_thread.h"

namespace Common {

class Event {
public:
    void Set() {
        std::scoped_lock lk{mutex};
        if (!is_set) {
            is_set = true;
            condvar.notify_one();
        }
    }

    void Wait() {
        std::unique_lock lk{mutex};
        condvar.wait(lk, [&] { return is_set.load(); });
        is_set = false;
    }

    bool WaitFor(const std::chrono::nanoseconds time);

    template<class Clock, class Duration>
    bool WaitUntil(const std::chrono::time_point<Clock, Duration> time) {
        std::unique_lock lk{mutex};
        if (!condvar.wait_until(lk, time, [this] { return is_set.load(); }))
            return false;
        is_set = false;
        return true;
    }

    void Reset() {
        std::unique_lock lk{mutex};
        // no other action required, since wait loops on the predicate and any lingering signal will
        // get cleared on the first iteration
        is_set = false;
    }

    [[nodiscard]] bool IsSet() const {
        return is_set;
    }

private:
    alignas(64) std::atomic<bool> is_set{false};
    std::condition_variable condvar;
    std::mutex mutex;
};

class Barrier {
public:
    explicit Barrier(std::size_t count_) : count(count_) {}

    /// Blocks until all "count" threads have called Sync()
    bool Sync(std::stop_token token = {}) {
        std::unique_lock lk{mutex};
        const std::size_t current_generation = generation;

        if (++waiting == count) {
            generation++;
            waiting = 0;
            condvar.notify_all();
            return true;
        } else {
            condvar.wait(lk, token,
                        [this, current_generation] { return current_generation != generation; });
            return !token.stop_requested();
        }
    }

private:
    std::condition_variable_any condvar;
    std::mutex mutex;
    std::size_t count;
    std::size_t waiting = 0;
    std::size_t generation = 0; // Incremented once each time the barrier is used
};

enum class ThreadPriority : u32 {
    Low = 0,
    Normal = 1,
    High = 2,
    VeryHigh = 3,
    Critical = 4,
};

enum class ThreadPlacement : u32 {
    Default = 0,
    Background = 1,
    Efficiency = 2,
};
/// Elastic 2:3:1 Non-SMT Core Partition Masks (Target: 6C/6T Intel Core i5-9600K @ 4.8 GHz)
enum class CorePartition : u64 {
    Unrestricted     = 0x3F, // All 6 physical cores (Cores 0–5)
    TimingAndAudio   = 0x01, // Core 0: HostTiming + MMCSS Pro Audio + FlexASIO (USB IRQ 0x01)
    VulkanSubmit     = 0x02, // Core 1: Primary Vulkan PipelineWorker Ring (GPU IRQ 0x02)
    PrimaryGuestJit  = 0x1C, // Cores 2–4: Biased Work-Stealing Common::Fiber JIT Pool
    SingleCoreJit    = 0x04, // Core 2: Dedicated Single-Core Fallback Anchor
    AuxQuarantine    = 0x20, // Core 5: Guest Core 3 + Async SPIR-V Shaders + QSV Feed (NVMe IRQ 0x20)
    ElasticGuestPool = 0x3C, // Cores 2–5: Full guest spillover envelope
};

/// Pins the calling thread to a specific physical core mask (no-op if host has < 6 cores).
bool SetCurrentThreadAffinityMask(u64 mask);
bool SetCurrentThreadPartition(CorePartition partition);

/// Hints a preferred physical home core to the OS scheduler within the thread's affinity mask
/// to preserve private L1d/L2 cache locality while still permitting work-stealing.
bool SetCurrentThreadIdealProcessor(u32 core_index);

/// RAII guard registering the calling thread with Windows MMCSS ("Pro Audio") for
/// deterministic sub-millisecond preemption over HostTiming on Core 0.
class ScopedProAudio {
public:
    ScopedProAudio();
    ~ScopedProAudio();

    ScopedProAudio(const ScopedProAudio&) = delete;
    ScopedProAudio& operator=(const ScopedProAudio&) = delete;

private:
    void* task_handle = nullptr;
};
void SetCurrentThreadPriority(ThreadPriority new_priority);
void SetCurrentThreadName(const char* name);
void SetCurrentThreadToPerformanceCores();
void SetCurrentThreadToEfficiencyCores();
void SetCurrentThreadToBackgroundWork();
void SetCurrentThreadToAllCores();

void RefreshThreadPolicies();
void PollThreadPolicies();

} // namespace Common

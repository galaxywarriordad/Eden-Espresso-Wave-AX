// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2022 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <chrono>

#include "audio_core/adsp/adsp.h"
#include "audio_core/audio_core.h"
#include "audio_core/renderer/system_manager.h"
#include "common/telemetry_types.hpp"
#include "common/thread.h"
#include "core/core.h"
#include "core/core_timing.h"

namespace AudioCore::Renderer {

SystemManager::SystemManager(Core::System& core_)
    : core{core_}, audio_renderer{core.AudioCore().ADSP().AudioRenderer()} {}

SystemManager::~SystemManager() {
    Stop();
}

void SystemManager::InitializeUnsafe() {
    if (!active) {
        active = true;
        audio_renderer.Start();
        thread = std::jthread([this](std::stop_token stop_token) {
            Common::SetCurrentThreadName("AudioRenderSystemManager");
            // Elastic 2:3:1 Core 0 Isolation + MMCSS "Pro Audio" Preemption Guard
            Common::SetCurrentThreadPartition(Common::CorePartition::TimingAndAudio);
            Common::SetCurrentThreadPriority(Common::ThreadPriority::Critical);
            Common::ScopedProAudio pro_audio_guard;

            while (active && !stop_token.stop_requested()) {
                u32 calculated_samples = 0;
                bool has_underrun = false;

                {
                    std::scoped_lock l{mutex1};
                    for (auto system : systems) {
                        system->SendCommandToDsp();
                        // Query the system's frame size configuration parameters safely
                        calculated_samples += system->GetSampleCount();
                        if (system->GetVoiceDropParameter() > 0.0f) {
                            has_underrun = true;
                        }
                    }
                }

                audio_renderer.Signal();
                audio_renderer.Wait();

                // =======================================================================
                // ðŸš€ INJECTING ASYNCHRONOUS ASIO TELEMETRY MATRIX (CORE 0 ISOLATION)
                // =======================================================================
                ::Eden::Core::Telemetry::g_metrics.asio_buffer_switches.fetch_add(
                    1, std::memory_order_relaxed);

                if (calculated_samples > 0) {
                    ::Eden::Core::Telemetry::g_metrics.audio_samples_popped.fetch_add(
                        calculated_samples, std::memory_order_relaxed);
                }

                if (has_underrun) {
                    ::Eden::Core::Telemetry::g_metrics.audio_underruns.fetch_add(
                        1, std::memory_order_relaxed);
                }
                // =======================================================================
            }
        });
    }
}

void SystemManager::Stop() {
    if (active) {
        active = false;
        thread.request_stop();
        thread.join();
        audio_renderer.Stop();
    }
}

bool SystemManager::Add(System& system_) {
    std::scoped_lock l2{mutex2};
    if (systems.size() + 1 > MaxRendererSessions) {
        LOG_ERROR(Service_Audio, "Maximum AudioRenderer Systems active, cannot add more!");
        return false;
    }
    {
        std::scoped_lock l{mutex1};
        if (systems.empty())
            InitializeUnsafe();
    }
    systems.push_back(&system_);
    return true;
}

bool SystemManager::Remove(System& system_) {
    std::scoped_lock l2{mutex2};
    {
        std::scoped_lock l{mutex1};
        if (systems.remove(&system_) == 0) {
            LOG_ERROR(Service_Audio, "Failed to remove a render system, it was not found in the list!");
            return false;
        }
    }

    if (systems.empty())
        Stop();
    return true;
}

} // namespace AudioCore::Renderer

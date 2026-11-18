// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <vector>
#include "common/common_types.h"

namespace Vulkan {

class Device;

struct PipelineCompileTask {
    std::function<void()> compile_functional_payload;
    u64 task_uid{0};
};

class PipelineWorkerPoolRing {
public:
    explicit PipelineWorkerPoolRing() noexcept;
    ~PipelineWorkerPoolRing();

    /**
     * @brief Boots up the asymmetrical compilation worker ring clamped strictly to 2 threads.
     * @param device Reference to the primary suitcase logical device.
     */
    void InitializeWorkerThreads(const Device& device);

    /**
     * @brief Pushes a new compilation task key onto the lock-free tracking ring boundary.
     * @param task Functional payload containing the driver pipeline generation pass.
     */
    void QueueCompileTask(PipelineCompileTask&& task);

    /**
     * @brief Waits until all queued compilation tasks finish or stop_loading is requested.
     */
    void WaitForRequests(std::stop_token stop_loading = {});

    /**
     * @brief Forces the queue to drain and halts all background processing.
     */
    void ShutdownPool();

    /**
     * @brief Queries the size of the active pending work backlog queue.
     */
    size_t GetPendingTaskCount() const noexcept {
        return pending_tasks_count.load(std::memory_order_relaxed);
    }

private:
    void WorkerThreadLoop(size_t thread_index) noexcept;

    std::atomic<bool> is_shutdown{false};
    const Device* logical_device{nullptr};
    
    // Master v1.3 Core 5 (0x20) Quarantine: Single Low-priority SPIR-V compiler worker
    std::vector<std::thread> background_workers;
    static constexpr size_t ASYNC_WORKER_CORE_CLAMP = 1;
    static constexpr size_t BACKLOG_CRITICAL_THRESHOLD = 8;

    // Execution Queue Tracking Structures
    std::queue<PipelineCompileTask> compile_queue;
    mutable std::mutex queue_mutex;
    std::condition_variable condition_variable;
    std::condition_variable drain_cv;
    
    // Atomic lock-free counter to insulate hot-path queries from mutex thumping
    std::atomic<size_t> pending_tasks_count{0};
};

} // namespace Vulkan
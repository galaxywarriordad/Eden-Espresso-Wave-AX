// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <windows.h>
#include <cstring> 
#include <fmt/format.h>
#include "common/logging.h"
#include "video_core/renderer_vulkan/vk_pipeline_worker.h"
#include "video_core/vulkan_common/vulkan_device.h"

// Standardized Vulkan external macro parameters to support standalone links
#ifndef VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
#define VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME "VK_KHR_push_descriptor"
#endif
#ifndef VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR 1000080000
#endif

namespace Vulkan {

PipelineWorkerPoolRing::PipelineWorkerPoolRing() noexcept = default;

PipelineWorkerPoolRing::~PipelineWorkerPoolRing() {
    ShutdownPool();
}

void PipelineWorkerPoolRing::InitializeWorkerThreads(const Device& device) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if (!background_workers.empty()) {
        return;
    }

    logical_device = &device;
    is_shutdown.store(false, std::memory_order_release);

    // =======================================================================
    // RUN LIVE PUSH DESCRIPTOR COMPLIANCE & CAPACITY VALIDATION
    // =======================================================================
    const vk::PhysicalDevice physical_device = device.GetPhysical();
    const std::vector<VkExtensionProperties> extensions =
        physical_device.EnumerateDeviceExtensionProperties();

    bool has_push_descriptor = false;
    for (const auto& ext : extensions) {
        if (std::strcmp(ext.extensionName, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) == 0) {
            has_push_descriptor = true;
            break;
        }
    }

    if (!has_push_descriptor) {
        LOG_ERROR(Render_Vulkan, "Vulkan Extension Check: CRITICAL: VK_KHR_push_descriptor is NOT supported by host driver!");
    } else {
        LOG_INFO(Render_Vulkan, "Vulkan Extension Check: SUCCESS: VK_KHR_push_descriptor is natively supported by host driver.");

        VkPhysicalDevicePushDescriptorPropertiesKHR push_properties{};
        push_properties.sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR);

        VkPhysicalDeviceProperties2 device_properties2{};
        device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_properties2.pNext = &push_properties;

        physical_device.GetProperties2(device_properties2);
        LOG_INFO(Render_Vulkan, "Vulkan Hardware Limits: Host reported maxPushDescriptors capacity is: {}.", push_properties.maxPushDescriptors);
    }
    // =======================================================================

    background_workers.reserve(ASYNC_WORKER_CORE_CLAMP);

    for (size_t i = 0; i < ASYNC_WORKER_CORE_CLAMP; ++i) {
        // FIX: Points directly to the correct synchronized thread routine member name
        background_workers.emplace_back(&PipelineWorkerPoolRing::WorkerThreadLoop, this, i);
    }

    LOG_INFO(Render_Vulkan, "Vulkan Async Pipeline Ring: Spawned {} background compilation threads locked to Cores 5-6.", ASYNC_WORKER_CORE_CLAMP);
}

void PipelineWorkerPoolRing::QueueCompileTask(PipelineCompileTask&& task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (is_shutdown.load(std::memory_order_relaxed)) {
            return;
        }
        compile_queue.push(std::move(task));
    }
    
    pending_tasks_count.fetch_add(1, std::memory_order_release);
    condition_variable.notify_one();
}

void PipelineWorkerPoolRing::WorkerThreadLoop(size_t thread_index) noexcept {
    std::string thread_name = fmt::format("VulkanPipelineWorker_{}", thread_index);
    LOG_INFO(Render_Vulkan, "Initializing background task scheduler thread: {}", thread_name);

    // Win32 Thread Priority Injection: Protect cores 1-4 from latency spiking
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    while (true) {
        // Master v1.3 Core 5 (0x20) Quarantine: Keep background shader workers strictly on Core 5
        // so Core 4 (0x10) remains 100% exclusive to Guest JIT CPUThread2.
        constexpr DWORD_PTR active_mask = 0x20;
        SetThreadAffinityMask(GetCurrentThread(), active_mask);

        PipelineCompileTask active_task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition_variable.wait(lock, [this]() { 
                return is_shutdown.load(std::memory_order_relaxed) || !compile_queue.empty(); 
            });

            if (is_shutdown.load(std::memory_order_relaxed) && compile_queue.empty()) {
                return;
            }

            active_task = std::move(compile_queue.front());
            compile_queue.pop();
        }

        if (active_task.compile_functional_payload) {
            try {
                active_task.compile_functional_payload();
            } catch (const std::exception& e) {
                LOG_ERROR(Render_Vulkan, "Vulkan Async Compiler: Task UID {} faulted: {}", active_task.task_uid, e.what());
            }
        }

        if (pending_tasks_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            drain_cv.notify_all();
        }
    }
}

void PipelineWorkerPoolRing::WaitForRequests(std::stop_token stop_loading) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    drain_cv.wait(lock, [this, &stop_loading]() {
        return pending_tasks_count.load(std::memory_order_acquire) == 0 ||
               is_shutdown.load(std::memory_order_relaxed) ||
               stop_loading.stop_requested();
    });
}

void PipelineWorkerPoolRing::ShutdownPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (is_shutdown.load(std::memory_order_relaxed)) {
            return;
        }
        is_shutdown.store(true, std::memory_order_release);
    }

    condition_variable.notify_all();

    for (std::thread& worker : background_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    background_workers.clear();
    
    std::unique_lock<std::mutex> lock(queue_mutex);
    std::queue<PipelineCompileTask>().swap(compile_queue); // Force atomic memory page releases cleanly
    pending_tasks_count.store(0, std::memory_order_release);
    LOG_INFO(Render_Vulkan, "Vulkan Async Pipeline Ring: Background thread pool dismantled cleanly.");
}

} // namespace Vulkan
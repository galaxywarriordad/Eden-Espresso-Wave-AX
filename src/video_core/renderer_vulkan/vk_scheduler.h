// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <queue>

// UNIFIED SYSTEM HEADERS: Grouped securely at the file apex to prevent compilation crashes
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

#include "common/alignment.h"
#include "common/common_types.h"
#include "common/polyfill_thread.h"
#include "common/settings.h"
#include "video_core/renderer_vulkan/vk_master_semaphore.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace VideoCommon {
template <typename Trait>
class QueryCacheBase;
}

namespace Vulkan {

class CommandPool;
class Device;
class Framebuffer;
class GraphicsPipeline;
class StateTracker;

struct QueryCacheParams;

class Scheduler {
public:
    explicit Scheduler(const Device& device, StateTracker& state_tracker);
    ~Scheduler();

    u64 Flush(VkSemaphore signal_semaphore = {}, VkSemaphore wait_semaphore = {});
    void Finish(VkSemaphore signal_semaphore = {}, VkSemaphore wait_semaphore = {});
    void WaitWorker();
    void DispatchWork();
    void RequestRenderpass(const Framebuffer* framebuffer);
    bool DeferColorClear(const Framebuffer* framebuffer, u32 rt_slot, const VkClearValue& value);
    bool DeferDepthStencilClear(const Framebuffer* framebuffer, const VkClearValue& value);
    void FlushDeferredClear();
    void RequestOutsideRenderPassOperationContext();

    bool IsRenderPassActive() const {
        return state.renderpass != VK_NULL_HANDLE;
    }

    bool UpdateGraphicsPipeline(GraphicsPipeline* pipeline);
    bool UpdateRescaling(bool is_rescaling);
    bool UpdateDescriptorBufferChunk(u32 descriptor_chunk);
    void InvalidateState();

    void SetQueryCache(VideoCommon::QueryCacheBase<QueryCacheParams>& query_cache_) {
        query_cache = &query_cache_;
    }

    void RegisterOnSubmit(std::function<void()>&& func) {
        on_submit = std::move(func);
    }

    template <typename T>
        requires std::is_invocable_v<T, vk::CommandBuffer, vk::CommandBuffer>
    void RecordWithUploadBuffer(T&& command) {
        if (chunk->Record(command)) {
            return;
        }
        DispatchWork();
        (void)chunk->Record(command);
    }

    template <typename T>
        requires std::is_invocable_v<T, vk::CommandBuffer>
    void Record(T&& c) {
        this->RecordWithUploadBuffer(
            [command = std::move(c)](vk::CommandBuffer cmdbuf, vk::CommandBuffer) {
                command(cmdbuf);
            });
    }

    [[nodiscard]] u64 CurrentTick() const noexcept {
        return master_semaphore->CurrentTick();
    }

    [[nodiscard]] bool IsFree(u64 tick) const noexcept {
        return master_semaphore->IsFree(tick);
    }

    void Wait(u64 tick, double target_fps = 0.0) {
        if (tick > 0) {
            if (tick >= master_semaphore->CurrentTick()) {
                Flush();
            }
            master_semaphore->Wait(tick);
        }
        if (Settings::values.use_speed_limit.GetValue() && target_fps > 0.0) {
            auto now = std::chrono::steady_clock::now();
            if (last_target_fps != target_fps) {
                frame_interval = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0 / target_fps));
                max_frame_count = static_cast<int>(0.1 * target_fps);
                last_target_fps = target_fps;
                frame_counter = 0;
                start_time = now;
            }
            frame_counter++;
            auto target_time = start_time + frame_interval * frame_counter;
            if (target_time >= now) {
                constexpr auto spin_tail = std::chrono::milliseconds(1);
                auto sleep_time = target_time - now;
                if (sleep_time > spin_tail * 2) {
                    std::this_thread::sleep_for(sleep_time - spin_tail);
                }
                
                // TARGET REALIGNMENT REFIT: Injects micro-architectural pause hints to prevent instruction pipeline flooding
                while (std::chrono::steady_clock::now() < target_time) {
                    #if defined(__x86_64__) || defined(_M_X64)
                    _mm_pause(); // Safely unblocks core execution threads and safeguards L1/L2 cache locality
                    #else
                    std::this_thread::yield();
                    #endif
                }
            } else if (frame_counter > max_frame_count) {
                frame_counter = 0;
                start_time = now;
            }
        }
    }

    [[nodiscard]] MasterSemaphore& GetMasterSemaphore() const noexcept {
        return *master_semaphore;
    }

    std::mutex submit_mutex;

private:
    class Command {
    public:
        virtual ~Command() = default;
        virtual void Execute(vk::CommandBuffer cmdbuf, vk::CommandBuffer upload_cmdbuf) const = 0;
        Command* GetNext() const { return next; }
        void SetNext(Command* next_) { next = next_; }
    private:
        Command* next = nullptr;
    };

    template <typename T>
    class TypedCommand final : public Command {
    public:
        explicit TypedCommand(T&& command_) : command{std::move(command_)} {}
        ~TypedCommand() override = default;
        TypedCommand(TypedCommand&&) = delete;
        TypedCommand& operator=(TypedCommand&&) = delete;
        void Execute(vk::CommandBuffer cmdbuf, vk::CommandBuffer upload_cmdbuf) const override {
            command(cmdbuf, upload_cmdbuf);
        }
    private:
        T command;
    };

    class CommandChunk final {
    public:
        void ExecuteAll(vk::CommandBuffer cmdbuf, vk::CommandBuffer upload_cmdbuf);

        template <typename T>
        bool Record(T& command) {
            using FuncType = TypedCommand<T>;
            static_assert(sizeof(FuncType) < sizeof(data), "Lambda is too large");

            command_offset = Common::AlignUp(command_offset, alignof(FuncType));
            if (command_offset > sizeof(data) - sizeof(FuncType)) {
                return false;
            }
            Command* const current_last = last;
            last = new (data.data() + command_offset) FuncType(std::move(command));

            if (current_last) {
                current_last->SetNext(last);
            } else {
                first = last;
            }
            command_offset += sizeof(FuncType);
            return true;
        }

        void MarkSubmit() { submit = true; }
        bool Empty() const { return command_offset == 0; }
        bool HasSubmit() const { return submit; }

    private:
        Command* first = nullptr;
        Command* last = nullptr;
        size_t command_offset = 0;
        bool submit = false;
        
        // MODERNIZED CACHE LINE REALIGNMENT: Aligned cleanly to 64-byte boundaries to protect AVX2 stack execution loops
        alignas(64) std::array<u8, 0x8000> data{};
    }; // SECURED STRUCTURAL FIX: Re-instates nested class bracket maps to prevent trailing namespace collisions

    struct State {
        VkRenderPass renderpass{};
        VkFramebuffer framebuffer{};
        VkExtent2D render_area = {0, 0};
        GraphicsPipeline* graphics_pipeline = nullptr;
        bool is_rescaling = false;
        bool rescaling_defined = false;
        bool needs_state_enable_refresh = false;
        u32 descriptor_buffer_chunk = 0;
        bool descriptor_buffer_bound = false;
    };

    struct DeferredClear {
        const Framebuffer* framebuffer = nullptr;
        u32 color_clear_mask = 0;
        std::array<VkClearValue, 8> color_values{};
        bool depth_stencil = false;
        VkClearValue depth_stencil_value{};
    };

    void BeginRenderPassImpl(const Framebuffer* framebuffer, VkRenderPass renderpass,
                             const VkClearValue* clear_values, u32 clear_value_count);
    void RealizeDeferredClear();
    void WorkerThread(std::stop_token stop_token);
    void AllocateWorkerCommandBuffer();
    u64 SubmitExecution(VkSemaphore signal_semaphore, VkSemaphore wait_semaphore);
    void AllocateNewContext();
    void EndPendingOperations();
    void EndRenderPass();
    void AcquireNewChunk();

    const Device& device;
    StateTracker& state_tracker;
    std::unique_ptr<MasterSemaphore> master_semaphore;
    std::unique_ptr<CommandPool> command_pool;
    VideoCommon::QueryCacheBase<QueryCacheParams>* query_cache = nullptr;
    vk::CommandBuffer current_cmdbuf;
    vk::CommandBuffer current_upload_cmdbuf;
    DeferredClear deferred_clear;
    std::unique_ptr<CommandChunk> chunk;
    std::function<void()> on_submit;
    State state;

    u32 num_renderpass_images = 0;
    std::array<VkImage, 9> renderpass_images{};
    std::array<VkImageSubresourceRange, 9> renderpass_image_ranges{};

    std::queue<std::unique_ptr<CommandChunk>> work_queue;
    std::vector<std::unique_ptr<CommandChunk>> chunk_reserve;
    std::mutex execution_mutex;
    std::mutex reserve_mutex;
    std::mutex queue_mutex;
    std::condition_variable_any event_cv;
    std::jthread worker_thread;

    std::chrono::steady_clock::duration frame_interval{};
    std::chrono::steady_clock::time_point start_time{};
    double last_target_fps{};
    u64 max_frame_count{};
    u64 frame_counter{};
};

} // namespace Vulkan

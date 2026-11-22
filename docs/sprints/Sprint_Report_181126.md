
📋 Sprint Progress Report — Vulkan 1.4 Async VRAM Shield & Cross-Host CMake Alignment (2026-11-19)


🎯 Executive Summary

This sprint resolved the synchronous VBLANK VRAM compaction bottleneck identified in the Adversarial Architectural Audit by implementing a Vulkan 1.4 asynchronous VK_QUEUE_TRANSFER_BIT eviction pipeline (VkVramShield) guarded by Synchronization2 split barriers (vkCmdSetEvent2 / vkCmdWaitEvents2) and a VK_KHR_timeline_semaphore-fenced background reaper pinned to Core 5 (ax::topology::Domain::Housekeeping). In parallel, cross-host CMake FetchContent resolution, ax::core target linkage, and Windows MMCSS priority guardrails were aligned across ax-sdk and Eden-Espresso-Wave-AX.

✅ Completed Deliverables


1. Proactive 5.00 GiB VRAM Pruning & Hysteresis State Machine (ax-sdk)

• Proactive 5.00 GiB Watermark (include/ax/vram.hpp): Introduced kPruneTriggerBytes (5.00 GiB / 5,368,709,120 bytes) ahead of the 5.50 GiB standard and 5.35 GiB LSFG hard ceilings, establishing a 0.35–0.50 GiB safety margin before Maxwell GM200 hits WDDM/NVIDIA system-RAM paging.
• Hysteresis & In-Flight Guard (EvaluateBudget): Added a ~4.50 GiB recovery low-water target and an eviction_in_flight atomic guard to prevent VBLANK frame-time oscillation and duplicate background transfer dispatches.
• Build System Verification (CMakeLists.txt): Verified ax::core and ax::sdk C++20 target exports with automatic Windows (winmm, avrt, psapi) and Linux (pthread, dl) platform linkage.

2. Cross-Host Build & Thread Priority Guardrails (Eden-Espresso-Wave-AX)

• Cross-Host CMake Wiring (externals/CMakeLists.txt & src/video_core/CMakeLists.txt): Added FetchContent_Declare(ax_sdk ...) pointing to the canonical GitHub ax-sdk host, gated asio_sdk and FlexASIO provisioning under if (WIN32) for clean Piccolo-AX Linux builds, linked ax::core into video_core, and registered renderer_vulkan/vk_vram_shield.hpp.
• Core 0 IRQ Starvation Guard (eden_engine.ini): Added explicit CoreRatio=2:3:1 domain scaling under [EngineTopology] and clamped HostTimingPriorityClass=HIGH so Core 0 MMCSS Pro Audio scheduling avoids starving HiBy FC3 USB xHCI interrupts.

3. Dedicated VK_QUEUE_TRANSFER_BIT & Vulkan 1.4 Split-Barrier Reaper (Eden-Espresso-Wave-AX)

• Dedicated Transfer Queue Discovery (vulkan_device.h & vulkan_device.cpp): Extended Vulkan::Device::SetupFamilies() and GetDeviceQueueCreateInfos() to detect dedicated DMA transfer families (VK_QUEUE_TRANSFER_BIT without VK_QUEUE_GRAPHICS_BIT) on Maxwell GM200 and modern GPUs, exposing GetTransferQueue(), GetTransferFamily(), and HasDedicatedTransferQueue() with automatic fallback to graphics_family.
• Vulkan 1.4 Synchronization2 Split Barriers (vulkan_wrapper.h & vulkan_wrapper.cpp): Added core Vulkan 1.3/1.4 vkCmdSetEvent2 and vkCmdWaitEvents2 dispatch pointers, dynamic loader hooks (with KHR extension fallbacks), and vk::CommandBuffer wrappers.
• Off-VBLANK Async VRAM Reaper (vk_vram_shield.hpp & vk_rasterizer.h/.cpp): Created Vulkan::VkVramShield and wired vram_shield.TickVblank(texture_cache, false) into RasterizerVulkan::TickFrame(), recording non-blocking ALL_GRAPHICS_BIT → TRANSFER_BIT split barriers (SetEvent2 / WaitEvents2) and offloading vmaDestroyImage / vkDestroyImage calls to a std::jthread worker pinned to Core 5 (Domain::Housekeeping).

🚀 Immediate Carry-Over Actions (Next Session)

1. Dynamic Silicon Topology Resolver (ax-sdk):
	• Replace the hw_threads > 6 stub in ax-sdk/src/topology.cpp with native Windows GetLogicalProcessorInformationEx (RelationProcessorCore + RelationCache) and Linux /sys/devices/system/cpu P-core/SMT/CCX detection, implementing the dynamic 2:3:1 proportional core-mask scaler and clean AvRevertMmThreadCharacteristics lifecycle management.
2. Cache-Line-Padded Batch SPSC Audio Ring (ax-sdk & Eden-Espresso-Wave-AX):
	• Upgrade LockFreeAudioQueue to a 64-byte cache-line-isolated PushBatch / PopBatch SPSC ring buffer (128-sample Win32 ASIO / 96-frame Piccolo ALSA) with per-callback telemetry updates, and isolate the standalone ASIO_main.cpp diagnostic probe from the production AudioCore sink.
3. Complete Cemu-Latte-AX & Piccolo-AX CMake/Buildroot Carry-Overs:
	• Append ax::core linkage to Cemu-Latte-AX/src/CMakeLists.txt and align Piccolo-AX/batocera-ax-external/package/cemu-latte-ax/cemu-latte-ax.mk CMake flags (-DENABLE_VCPKG=OFF, -DALLOW_PORTABLE=OFF, sdl3, wxwidgets).

# ☕ Eden Espresso Wave [AX] — Technical Specification

**Bleeding-Edge Telemetry & Hardware Instrumentation Architecture**

---

## 📑 Table of Contents

* [1. Introduction: Bringing Bleeding-Edge Instrumentation to Eden](#-1-introduction-bringing-bleeding-edge-instrumentation-to-eden)
* [2. Current Architecture & Implementation Guide](#️-2-current-architecture--implementation-guide)
* [3. Roadmap to Elite Territory: Upcoming Architectural Improvements](#️-3-roadmap-to-elite-territory-upcoming-architectural-improvements)

---

## 🌐 1. Introduction: Bringing Bleeding-Edge Instrumentation to Eden

Emulating complex console architectures on commodity desktop hardware—specifically an asymmetrical core topology (Intel i5-9600K) paired with a discrete Maxwell GPU (NVIDIA GTX 980 Ti)—presents formidable observability challenges. Traditional developer tools, such as external third-party overlays (e.g., RTSS, MangoHud) or heavyweight profilers, introduce measurable polling overhead, thread contention, and micro-stutters that distort the very metrics they attempt to measure. Furthermore, they lack domain-specific awareness of guest JIT compilation loops, Vulkan command buffer submission latencies, and audio subsystem buffer health.

To overcome these limitations, the **Eden Espresso Wave [AX]** initiative set out to integrate a production-grade, non-stuttering, native Vulkan On-Screen Display (OSD) and hardware telemetry subsystem directly into the engine's core render loop (`telemetry_osd.cpp`). By marrying native OS memory polling, hardware-accelerated Vulkan query pools, strict asymmetrical thread pinning, cross-process shared-memory export (`ax-sdk` `v1_36_1`), and a multi-stage Profile-Guided Optimization (PGO) pipeline, Eden achieves console-parity observability—bridging the gap between open-source emulation and professional console development kits like Microsoft PIX or the PlayStation Performance Analyzer.

---

## 🏗️ 2. Current Architecture & Implementation Guide

The deployed telemetry and instrumentation stack is built upon several tightly integrated subsystems designed for zero runtime overhead and surgical precision.

### 2.1 Elastic Core Topology (`2:3:1`) & Thread Affinity
To prevent context-switching jitter and cache thrashing, Eden enforces strict thread affinity across the Intel i5-9600K's 6 physical cores:

* **Core 0 (`0x01`):** Dedicated to the Audio Render System Manager and HiBy FC3 ASIO / ALSA ring buffer (`ax::audio::LockFreeStereoRing`), ensuring zero audio underruns.
* **Core 1 (`0x02`):** Dedicated Vulkan Worker handling command buffer dispatch and primary graphics submission.
* **Cores 2–4 (`0x1C`):** Allocated to guest JIT execution fibers and CPU emulation loops.
* **Core 5 (`0x20`):** Dedicated to the QSV / VA-API Video Feeder, FFmpeg decode fallback worker pool (`execute2`), and background SPIR-V compilation tasks.

### 2.2 Native Vulkan Query Pools & Pipeline Statistics
Rather than estimating GPU workload via blunt high-level timers, Eden leverages native Vulkan query pools configured with `VK_QUERY_TYPE_PIPELINE_STATISTICS` (`telemetry_osd.cpp`):

```cpp
VkQueryPoolCreateInfo query_pool_info{
    .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
    .queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
    .queryCount = 1,
    .pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                          VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                          VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
                          VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                          VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
};
vkCreateQueryPool(device_, &query_pool_info, nullptr, &query_pool_);
```

Results are retrieved asynchronously at the end of each frame using `vkGetQueryPoolResults` with `VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_PARTIAL_BIT` flags, feeding vertex, primitive, and shader invocation counters directly into the OSD snapshot buffer without stalling the GPU pipeline.

### 2.3 Win32 Process Memory Polling
To monitor host resource consumption without external API overhead (`telemetry_osd.cpp`), Windows process memory counters are queried natively using the `psapi` header:

```cpp
#include <windows.h>
#include <psapi.h>

PROCESS_MEMORY_COUNTERS pmc;
if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    current_snapshot_.working_set_mb = pmc.WorkingSetSize / (1024 * 1024);
    current_snapshot_.commit_size_mb = pmc.PagefileUsage / (1024 * 1024);
}
```

### 2.4 SPIR-V Font Shaders & OSD Render Pipeline
The OSD renders crisp telemetry text directly into the active swapchain render pass using custom GLSL font shaders compiled to binary SPIR-V modules (`font.vert.spv`, `font.frag.spv` as referenced in `WhatsNext.md`):

* **Vertex Layout:** Custom `TextVertex` structure (`telemetry_osd.h`) containing normalized screen-space coordinates (`float position[2]`, range `-1.0` to `1.0`) and texture UV coordinates (`float uv[2]`).
* **Pipeline State Object (PSO):** Configured with triangle list topology, alpha blending enabled (`VK_BLEND_FACTOR_SRC_ALPHA` and `VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA`), dynamic viewport/scissor states, and zero push-descriptor overhead.
* **Vertex Buffer Staging:** Per-frame text strings (`telemetry_osd.cpp`) are formatted, converted into character quad vertices (2 triangles / 6 vertices per character), mapped to host-visible memory, and drawn via `vkCmdDraw` inside the active render pass.

### 2.5 Multi-Stage Profile-Guided Optimization (PGO)
Eden utilizes a rigorous 3-stage PGO build pipeline to linearize hot-path code blocks and optimize instruction cache locality:

* **Stage 0LD (`PGO_stage0LD.bat`):** Compiles the modified baseline code and verifies basic telemetry and thread affinities.
* **Stage 1 (`PGO_stage1.bat`):** Builds an instrumented binary and executes standardized 15-minute training sweeps across base (`v0`) test titles while tracking runtime telemetry.
* **Stage 2 (`PGO_stage2.bat`):** Merges profiling data (`.pgc` / `.profraw` → `default.profdata`) via `Scripts\validate_profile.ps1` and compiles the final, L1i-linearized high-performance production binary.

### 2.6 Cross-Process Shared-Memory Export (`ax::telemetry` `v1_36_1`)
In addition to rendering the in-swapchain Vulkan OSD, `telemetry_osd.cpp` exports 64-byte cache-line-aligned atomic counters (`ax::TelemetryCounters`) to `Local\AX_Telemetry` (Windows) and `/dev/shm/ax_telemetry` (Linux) via `ax::InitTelemetryShm()`—allowing out-of-process monitors (`ax-sdk/tools/track_telemetry.ps1` and `ax-top`) to inspect GPU pipeline statistics, VRAM shield headroom (`5.00 GiB` prune / `5.50 GiB` ceiling), and `min / P99 / max` microsecond audio callback jitter (`ax::RecordAudioCallbackMetrics`) across `Eden`, `Cemu`, `Generation-X-AX`, and `FLAC_AX` with zero IPC locks.

---

## 🗺️ 3. Roadmap to Elite Territory: Upcoming Architectural Improvements

To push Eden from advanced emulation instrumentation into absolute elite industry territory, the upcoming development cycles introduce four major telemetry and optimization pillars:

* **3.1 Vendor-Specific VRAM & Bus Saturation Profiling (NVIDIA NVAPI / ADL)**
  * *Objective:* Bypass generic OS memory counters to poll true physical VRAM allocation, memory bus bandwidth utilization, and L2 cache hit/miss rates directly from the Maxwell GTX 980 Ti hardware performance registers.
* **3.2 Telemetry-Guided PGO Training Sweeps (`PGO_automated_sweep.py` / `ax::telemetry`)**
  * *Objective:* Gate Stage 1 LLVM `-fprofile-generate` instrumentation (`__llvm_profile_reset_counters()` / `__llvm_profile_dump()`) using live Vulkan pipeline statistics (`VK_QUERY_TYPE_PIPELINE_STATISTICS`) and `Local\AX_Telemetry` gameplay-state flags—excluding cold menu/loading screens so `default.profdata` linearizes only genuine 60/120 FPS gameplay, Vulkan submission, and audio hot paths across `Eden`, `Cemu`, `Generation-X-AX`, and `FLAC_AX`.
* **3.3 Asynchronous Telemetry Stream (JSON / CSV Ring-Buffer Dumps)**
  * *Objective:* Implement a zero-cost background ring-buffer serializer that flushes historical telemetry snapshots (frame-time variance, JIT spikes, core execution times) to disk upon session termination for automated regression graphing and performance tracking.
* **3.4 JIT Compaction & Cache Fragmentation Metrics**
  * *Objective:* Introduce real-time tracking for JIT allocation pools, translation latency per instruction block, and code cache eviction rates to instantly detect and eliminate micro-stutters during intensive shader compilation phases.

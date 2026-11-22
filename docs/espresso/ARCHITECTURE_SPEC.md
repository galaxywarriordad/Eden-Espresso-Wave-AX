# 🗺️ Eden Espresso Wave `[AX]`: Comprehensive Architectural Specification (Master v1.35.1)

Root Subsystem Target Topology, Multi-Threaded Data-Flow, Operational Context, & Global Microarchitectural Dependency Matrix  
**Updated:** `2026-11-21` (Post-`ax-sdk v1_36_1` Shared-Memory Telemetry, Vulkan 1.4 Async `VK_QUEUE_TRANSFER_BIT` VRAM Shield & 7-Repository Ecosystem Alignment)

**Hardware Baseline:** Intel Core i5-9600K (6 Physical Cores / 6 Hardware Threads / Zero SMT — `x48` / `4.8 GHz` locked symmetrically across all cores, 9 MB L3 Smart Cache) + NVIDIA GeForce GTX 980 Ti (6 GB GDDR5, Maxwell GM200, Vulkan 1.4) + Intel UHD Graphics 630 iGPU (Vendor `0x8086`, `VK_API_VERSION_1_3` capped) + HiBy FC3 USB DAC (ESS Sabre ES9281AC Pro) + Akai MPK Mini MK3 (`MPK mini 3` USB-MIDI)

**Target Platforms:** Windows 10/11 Desktop Host (`PGO_stage0.bat` / `PGO_stage0LD.bat` → `PGO_stage1.bat` → `PGO_stage2.bat`) & Piccolo `[AX]` (`< 512 MB` `toram` EROFS `PREEMPT_RT` Linux Console OS + opt-in `< 35 MB` `piccolo-ax-desktop`)

**Storage & USB Bus Topology:**
* **NVMe #1:** OS + `eden.exe` + Vulkan Shader Cache
* **NVMe #2:** `< 200 MB` Updates & DLCs
* **7200 RPM SATA HDD:** Level-18 Block-Compressed `.chd` / `.ncz` Main Game Images
* **Split USB Root Hubs:** HiBy FC3 USB 2.0 Audio DAC (`hw:CARD=FC3,DEV=0`) on rear Z390 PCH root hub → `Core 0` (`0x01`) xHCI IRQ; Akai MPK Mini MK3 USB 1.1 Full-Speed MIDI (`hw:CARD=MPKmini3,DEV=0`) on secondary/front port → `Core 5` (`0x20`) RawMIDI poll

---

## 📑 Table of Contents

* [Section 1: Unified Macro Data-Flow, PGO Workspace Automation, & Host Runtime Topology](#-section-1-unified-macro-data-flow-pgo-workspace-automation--host-runtime-topology)
* [Section 2: Host Memory Virtualization, GMMU Page Translation, & Cache-Coherency Tracking](#️-section-2-host-memory-virtualization-gmmu-page-translation--cache-coherency-tracking)
* [Section 3: Asynchronous CPU Execution, Fiber Thread Dispatching, & Temporal Pacing](#️-section-3-asynchronous-cpu-execution-fiber-thread-dispatching--temporal-pacing)
* [Section 4: Real-Time Audio Scheduling, Graph Compilation, & FlexASIO / ALSA Silicon Offloading](#-section-4-real-time-audio-scheduling-graph-compilation--flexasio--alsa-silicon-offloading)
* [Section 5: Real-Time Video, Telemetry OSD, Storage/Shader Quarantine, GTX 980 Ti Texture Compression Policy, & Telemetry](#-section-5-real-time-video-telemetry-osd-storageshader-quarantine-gtx-980-ti-texture-compression-policy--telemetry)
* [Section 6: Consolidated Upstream PR Triage & Silicon Adaptation Matrix](#-section-6-consolidated-upstream-pr-triage--silicon-adaptation-matrix)
* [Section 7: Expanded 6-Phase Linear Engineering Roadmap](#️-section-7-expanded-6-phase-linear-engineering-roadmap)

---

## 🌐 Section 1: Unified Macro Data-Flow, PGO Workspace Automation, & Host Runtime Topology

```text
========================================================================================================================
                   EDEN ESPRESSO WAVE [AX] v1.35.1 ELASTIC 2:3:1 NON-SMT + STORAGE-AWARE TOPOLOGY
========================================================================================================================

  [ BATCH GENERATION CONTROL ]      [ DEVELOPER INFRASTRUCTURE ]       [ ASYNC THREAD ENGINE ]    [ SILICON PRESENTATION ]
  
  PGO_stage0LD / 1 / 2.bat            CMakeLists.txt (Externals)          Core::DeviceMemory      NVIDIA GTX 980 Ti (dGPU)
                │                               │                              │ (512GB Reserve)     (Vulkan Graphic Pipes)
                ▼                               ▼                              ▼                            ▲
       chcp 65001 (UTF-8 Stream)      Policy CMP0169 Gate enforced     Core::Memory::Memory                 │ Mask: 0x02
                │                               │                      (_bextr_u64 3-Arg BMI1)              │ (Core 1)
                ▼                               ▼                      (SeLockMemoryPrivilege)              │
      Intelligent Delta Triage        FetchContent_Declare(ax_sdk)             │                            │
      Purges Stale Engine Handles     (v1_36_1) + asio_sdk (WIN32)             ▼                            │
                │                               │                      GPUDirtyMemoryManager ───────► [ Vulkan Core ]
                ▼                               ▼                      (Lock-Free Weak-CAS)         - VulkanWorker Ring
  %AX_TOOLS_DIR%\check_hardware.ps1    Export Targets: ax::core,               │                      - Pinned: Core 1 (0x02)
  (via resolve_ax_tools.bat)           asio::asio                              ▼                      - GTX 980 Ti IRQ: 0x02
                │                               │                      Tegra::GMMU Manager            - Page-Guarded UBO Stream
                ▼                               ▼                      (Coalesced Range-Locks)        - 5.00 GiB Async VRAM Shield
  Scripts\macro_update.ps1            target_link_libraries()                  │                      - VK_QUEUE_TRANSFER_BIT
  + Scripts\shader_validate.ps1       (Pure Win32 + QSV; No CUDA)              ▼
  (GLSL 460 + glslangValidator)                 │                      Core::CpuManager
                │                               │                      (Config-Driven 2:3:1 Scaler)
                ▼                               │                      (ax_engine.ini / ax::config)
       High-Velocity Compiler Pass ─────────────┘                              │
     (clang-cl 23.1.4 + ThinLTO)                                               │
                │                                                              │
                ▼                                                              │
  %AX_TOOLS_DIR%\track_telemetry.ps1 ───────────┬──────────────────────────────┘
  (SHM Local\AX_Telemetry + Regression Gate)    │
       ┌────────────────────────────────────────┼────────────────────────────────────────┐
       ▼                                        ▼                                        ▼
[ AUXILIARY QUARANTINE: CORE 5 ]     [ PRIMARY GUEST JIT: CORES 2–4 ]         [ AUDIO CORE INTERCONNECT ]
- Mask: 0x20 (Quarantine Sink)       - Mask: 0x1C (Home-Biased Fibers)        - AudioCore::AudioCore Coordinator
- Guest Core 3 (Normal | 0x3C Spill)   - Guest ARM Cores 0–2 (High Priority)               │
- Main GUI / Input / MPK3 MIDI         - Home Bias: C0►0x04, C1►0x08, C2►0x10    ┌─────────┴─────────┐
- CHD / ScopedNczQuarantine (BelowN)   - Shielded from CHD/Shaders/QSV/IRQs      ▼                   ▼
- QSVVideoFeeder / VRAM Reaper                                          [ AudioManager ]     [ SystemManager ]
- PipelineWorkerPoolRing x1 (Low)                                       (Lock-Free Atomics)  (MMCSS / SCHED_FIFO 99)
- AVX2 ZSTD Cache Decomp (Non-Temp)                                              │                   │
- Storage IRQs: SATA AHCI + NVMe x2 (0x20)                                       ▼                   ▼
       │                                                                [ AudioCore::Event ] [ CommandGenerator ]
       ▼                                                                (std::ranges Check)  (Polymorphic Layouts)
[ REAL-TIME VIDEO SUBSYSTEM ]                                                   │                   │
- Tegra::Host1x::Host1x Engine                                                   ▼                   ▼
       │                                                                [ Sink Endpoints ]   [ CommandBuffer ]
 ┌─────┴─────┐                                                          - FlexASIO / ALSA    - std::construct_at
 ▼           ▼                                                          (Direct Exclusive)   - 0xCAFEBABE Magic
[ Vic ]  [ Tegra::Nvdec ]                                               - WASAPI / hw:FC3    - Time Estimator
(Inline) (Naked Variant Tables)                                         - Cubeb Secondary    - Hardened Biquad (PR #2)
 │           │                                                                   │                   │
 ▼           ▼                                                                   └─────────┬─────────┘
[Raster] [ ::Tegra::Decoder ]                                                              ▼
 │           │                                                                  Host Core 0 Loop (Mask: 0x01)
 │     ┌─────┴─────┐                                                            - 4.8 GHz BIOS x48 Lock
 │     ▼           ▼                                                            - HostTiming (High | 0x20 Fallback)
 │ codecs/vp9  codecs/vp8                                                       - HiBy FC3 USB xHCI IRQ: 0x01
 │ (Intel QSV) (Intel QSV)                                                                 │
 │     │           │                                                                       ▼
 │     └─────┬─────┘                                                            [ HiBy FC3 USB DAC ]
 │           ▼                                                                  (ESS ES9281AC Pro Clock)
 └──► Intel UHD 630 iGPU (Vendor 0x8086)
      (NT Shared Handle / VA-API DMA-BUF | QSVVideoFeeder: Core 5 / 0x20)
```

### ⚙️ Telemetry Build System & Host Silicon Execution Blueprint

* **The Character Encoding Firewall & Synchronous Workspace Purge (`PGO_stage0LD.bat` & `clean_all.ps1 -Wait`):** Forces the host context interpreter straight into `chcp 65001` (UTF-8 Code Page Mode), preserving Unicode structure tags of the medium-shade coffee liquid asset (`▒`) without character dropping or line buffer splitting. Terminates active build engine processes, clears telemetry metrics logs, purges internal binary tracking manifests, and executes a synchronous (`-Wait`) multi-threaded PowerShell disk deletion worker prior to running `clang-cl 23.1+` via Ninja.
* **Shared `ax-sdk/tools/` Delegation (`Scripts\resolve_ax_tools.bat` & `PGO_config.bat`):** Resolves `%AX_TOOLS_DIR%` across `%AX_SDK_DIR%\tools`, sibling `..\ax-sdk\tools`, and CMake `FetchContent` caches so `Eden` shares `Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1` (`-Revert`), `track_telemetry.ps1`, `check_regression.ps1`, and `clean_all.ps1` (`-Wait`) without duplicating host scripts.
* **Integrated PGO Stage Quality Gates (`PGO_stage0.bat`, `PGO_stage0LD.bat`, `PGO_stage1.bat`, `PGO_stage2.bat`):** All PGO stage batch scripts invoke `Scripts\macro_update.ps1`, `Scripts\shader_validate.ps1`, and `%AX_TOOLS_DIR%\track_telemetry.ps1` + `check_regression.ps1` gates between cleaning and compilation/baking steps. PowerShell invocations use scoped inline `-ExecutionPolicy Bypass` flags, and batch conditional blocks avoid nested multi-line `else` syntax hazards via explicit error-level checks and `setlocal EnableDelayedExpansion`.
* **Interactive Root Configurator Menu (`PGO_config.bat`):** Queries and renders live IRQ affinity audit results sourced from `%AX_TOOLS_DIR%\check_hardware.ps1` directly inside the CLI menu to verify system readiness before launching builds or sweeps.
* **Cross-Host `ax-sdk` (`v1_36_1`) Ingestion & `WIN32`-Gated Audio Provisioning (`CMakeLists.txt`, `src/audio_core/CMakeLists.txt`):** Fetches the shared MIT-licensed C++20 silicon core (`ax-sdk` `v1_36_1` → `ax::core`, linked publicly to `common`/`video_core` and privately to `audio_core`) via CMake `FetchContent`, while gating Windows-only `asio_sdk` (`audiosdk/asio.git`) and `FlexASIO` provisioning strictly under `if (WIN32)` so Linux/Piccolo builds compile cleanly without Win32 COM dependencies.
* **v1.36.1 Linker, Unified Settings & Shared-Memory Telemetry Alignment:** Validates PGO `.profdata` counters via `Scripts\validate_profile.ps1` ahead of Stage 2 compilation, enforces `HostTimingPriorityClass=HIGH` alongside MMCSS `Pro Audio` in `ax_engine.ini`, stages `ax_engine.ini` + `FlexASIO.toml` automatically into `%BUILD_DIR%\bin\`, and publishes zero-overhead atomic metrics to `Local\AX_Telemetry` (Windows) / `/dev/shm/ax_telemetry` (Linux).
* **Dynamic `2:3:1` Physical-Core Partitioning (`ax::topology` & `ax_engine.ini`):** Enumerates physical P-cores via `GetLogicalProcessorInformationEx` (`RelationProcessorCore` + `EfficiencyClass`) on Windows and `/sys/devices/system/cpu` (`thread_siblings_list` + `cpu_capacity`) on Linux—filtering out SMT hyperthreads and E-cores, producing exact `0x01`/`0x02`/`0x1C`/`0x20` masks on the i5-9600K, and scaling `CoreRatio=2:3:1` proportionally across 8/12/16-core CPUs with RAII `AvRevertMmThreadCharacteristics` cleanup.
* **1:1 PCIe/USB/SATA Hardware IRQ Steering & DAC Arbitration:** Configures reversible static MSI IRQ steering (`%AX_TOOLS_DIR%\set_irq_affinity.ps1` / `-Revert`) alongside process-scoped runtime priority isolation, and coordinates exclusive `hw:CARD=FC3,DEV=0` DAC handoff with `FLAC_AX` (`flac_ax` / `ax_av_studio`) on `Piccolo [AX]` via `/run/lock/ax-dac.lock` (`SIGUSR1`).

---

## 🗃️ Section 2: Host Memory Virtualization, GMMU Page Translation, & Cache-Coherency Tracking

**Component Cluster:** `bmi_util.h`, `device_memory.h/.cpp`, `device_memory_manager.h/.cpp`, `gpu_dirty_memory_manager.h`, `memory.h/.cpp`

### 🔍 Microarchitectural Status Vetting & Technical Reality

* **Linear Contiguous Address Space (`Core::DeviceMemory`):** Reserves a 512 GB contiguous virtual memory space (`VirtualReserveSize = 1ULL << 39`) inside the host OS environment during standard recompiler execution. Guest application memory addresses map linearly to host pointers via plain pointer offsets, bypassing expensive map lookups, hash checks, or dynamic vector indirection loops.
* **Large-Page / HugeTLB Backing (`EnableLockMemoryPrivilege` & `ax::memory`):** Queries `OpenProcessToken` and `LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)` during `Memory::Impl` construction on Windows to enable `SeLockMemoryPrivilege`, and maps $6 \times 1\text{G}$ (6.0 GB) $+ 1024 \times 2\text{M}$ (2.0 GB) `MAP_HUGETLB` + `mlockall()` arenas on Piccolo `[AX]`, minimizing Skylake DTLB misses across guest address ranges.
* **Single-Cycle 3-Argument BMI1 Intrinsic Translation (`bmi_util.h` & `memory.cpp`):** Inside `Common::BitUtils::ExtractBitField`, `GetPointerFromRasterizerCachedMemory`, and `GetPointerFromDebugMemory`, page-index extraction invokes the explicit 3-argument Coffee Lake BMI1 intrinsic `_bextr_u64(vaddr, static_cast<u8>(YUZU_PAGEBITS), 36)` (`HAS_BMI1 = 1` under `-march=skylake`). This compiles directly to the single-cycle x86-64 `BEXTR` instruction.
* **Split-Path GMMU Page Table Resolution (`Core::DeviceMemoryManager`):** Hooks the backing store directly into graphics page table layers to accommodate games that alias a single physical texture buffer or vertex array across multiple distinct virtual address handles (`MULTI_FLAG` mode redirects multi-aliased textures to a dedicated `MultiAddressContainer` chain).
* **Lock-Free Weak-CAS Dirty Tracking (`Core::GPUDirtyMemoryManager::Collect`):** Packs the page address and sub-page modification bitmask into a single 8-byte `TransformAddress` and applies a lock-free weak Compare-And-Swap loop (`compare_exchange_weak` with `std::memory_order_acq_rel`), allowing parallel execution threads to merge dirty snapshots with zero thread-scheduling latency.
* **Hardware Bit-Scan Coherency Extraction (`Gather()`):** Executed directly on Core 1 (`0x02`) alongside Vulkan command submission. Swaps internal collection array pointers instantly (`front_buffer.swap(back_buffer)`), then processes snapshots using C++20 bit-manipulation primitives (`std::countr_zero` / `std::countr_one` / `ClearLowestSetBit` → `TZCNT` / `BLSR`) to extract continuous dirty byte lengths in a few clock cycles.

---

## ⏱️ Section 3: Asynchronous CPU Execution, Fiber Thread Dispatching, & Temporal Pacing

**Component Cluster:** `core_timing.h/.cpp`, `cpu_manager.h/.cpp`, `speed_limiter.h/.cpp`

### 🔍 Microarchitectural Status Vetting & Technical Reality

* **User-Mode Fiber Context Switching (`Core::CpuManager`):** Maps emulated ARM cores onto host processor tracks using User-Mode Fibers (`Common::Fiber` via `PhysicalCore::RunThread` and `KInterruptManager::HandleInterrupt`), eliminating OS kernel-mode context switches and keeping host CPU instruction caches populated.
* **Home-Biased Elastic 2:3:1 Guest Scheduling (`Core::CpuManager::RunThread`):**
  * **Guest ARM Cores 0–2 (`PrimaryGuestJit = 0x1C` / `ThreadPriority::High`):** Scheduled inside a home-biased `Common::Fiber` pool with 1:1 ideal-processor affinity biasing (Guest Core 0 → Core 2 `0x04`, Guest Core 1 → Core 3 `0x08`, Guest Core 2 → Core 4 `0x10`) before stealing across `0x1C`. Preserves private 32 KB L1d and 256 KB L2 caches while remaining 100% shielded from `.chd`/`.ncz` decompression, shaders, QSV/VA-API, and storage IRQs.
  * **Guest ARM Core 3 (`ElasticGuestPool = 0x3C` / `ThreadPriority::Normal`):** Biased to Core 5 (`0x20`) via `SetCurrentThreadIdealProcessor(5)` to run lightweight Horizon OS services, while retaining `0x3C` elastic spillover onto Cores 2–4 so cross-core IPC never stalls during SATA AHCI + Level-18 decompression bursts.
* **Lock-Free Single-Core Preemption & Sequentially Consistent Page-Table Fences (`PreemptSingleCore` & `Memory::SetCurrentPageTable`):** Rotates `current_core` via a lock-free Compare-And-Swap loop, anchors single-core workloads onto Host Core 2 (`SingleCoreJit = 0x04`), and enforces `std::atomic_thread_fence(std::memory_order_seq_cst)` barriers across page-table transitions.
* **$O(1)$ Amortized Fibonacci Heap Timeline (`Core::Timing::CoreTiming`):** Stores scheduled sub-device callbacks inside a Boost Fibonacci Heap (`heap_t`), providing $O(1)$ amortized complexity for insertions and key updates.
* **Deterministic Core 0 Yielding & Symmetrical Clock Execution:** Pins `HostTiming` to Core 0 (`TimingAndAudio = 0x01`) at `ThreadPriority::High` under a locked 4.8 GHz frequency baseline. This structurally eliminates cross-core multiplier-step adjustments during fiber handoffs to the primary JIT pool (`0x1C`). It deterministically yields to the Critical MMCSS / `SCHED_FIFO 99` audio engine, synchronised through `std::scoped_lock lock{advance_lock, basic_lock}` to eliminate lock-order inversion deadlocks.
* **Hybrid High-Resolution Frame Pacing (`SpeedLimiter`, `timeBeginPeriod(1)` & `clock_nanosleep`):** Invokes `timeBeginPeriod(1)` on `Initialize` and pairs `CreateWaitableTimerExW` (or `clock_nanosleep` `TIMER_ABSTIME` on Piccolo) down to $\sim 1.2\text{ ms}$ before target presentation with a `_mm_pause()` spin-wait tail for $< 1\text{ µs}$ frametime delivery.

---

## 🔊 Section 4: Real-Time Audio Scheduling, Graph Compilation, & FlexASIO / ALSA Silicon Offloading

**Component Cluster:** `audio_core.h/.cpp`, `audio_manager.h/.cpp`, `audio_event.h/.cpp`, `audio_render_manager.h/.cpp`, `system_manager.h/.cpp`, `system.h/.cpp`, `command_generator.h/.cpp`, `command_buffer.h/.cpp`, `command_processing_time_estimator.h/.cpp`, `decoder.h/.cpp`, `decoder_manager.h/.cpp`, `hardware_opus.h/.cpp`, `flex_asio_manager.hpp/.cpp`, `lockfree_audio_queue.hpp`, `sink_stream.cpp`, `cubeb_sink.cpp`, `sdl3_sink.cpp`

### 🔍 Microarchitectural Status Vetting & Technical Reality

* **Inline Cache-Local Instantiation (`AudioCore::AudioCore`):** Instantiates `AudioManager` and the ADSP microkernel inline via `std::optional` to maintain strict L1/L2 cache locality on Host Core 0, operating at a stable 4.8 GHz BIOS `x48` baseline. This guarantees absolute timing uniformity across zero-heap graph compilation sequences.
* **MMCSS Pro Audio / `SCHED_FIFO 99` Preemption Guard & Shared-Memory Jitter Telemetry (`SystemManager` & `SinkStream`):** Pins `AudioRenderSystemManager` onto Core 0 (`TimingAndAudio = 0x01`) at `ThreadPriority::Critical` wrapped in `ScopedProAudio` (`AvSetMmThreadCharacteristicsW` / `SCHED_FIFO` priority 99). Inside `SinkStream::ProcessAudioOutAndRender()`, each hardware buffer callback records buffer switches, queue underruns (`audio_underruns`), and microsecond callback latency (`min / P99 / max us` via `ax::RecordAudioCallbackMetrics()`) directly into cross-process shared memory (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`).
* **Cache-Line-Isolated Batch SPSC PCM Ring Buffer (`ax::audio` & `lockfree_audio_queue.hpp`):** Delegates stereo frame transport to `ax-sdk`'s 64-byte cache-line-padded `ax::audio::LockFreeStereoRing<2048, float>` (`PushBatch`, `PopBatch`, and `PopPlanarBatch` for separate Left/Right Steinberg ASIO buffers), using contiguous `std::memcpy` wrap-around slices and per-callback telemetry updates inside a 128-sample (`2.67 ms` Windows) / 96-frame (`2.00 ms` Piccolo ALSA) window.
* **Config-Driven Primary Hardware Path + Secondary Cubeb / SDL3 Fallback (`FlexASIOManager`):**
  * **Windows Primary Path:** `FlexASIOManager::ProvisionLocalConfiguration()` reads `AsioBufferSamples` (`128`) and `ExclusiveDeviceName` (`"HiBy USB Audio"`) from `ax_engine.ini` via `ax::LoadEngineConfig()`, initializes `Local\AX_Telemetry` via `ax::InitTelemetryShm()`, and writes synchronized `FlexASIO.toml` profiles (`wasapiExclusiveMode = true`, `wasapiAutoConvert = false`) across the execution directory and `%USERPROFILE%`, routing bit-perfect PCM over USB xHCI (IRQ `0x01`) directly to the ESS Sabre ES9281AC Pro clock inside the HiBy FC3 USB DAC.
  * **Piccolo Linux Primary Path:** Opens direct exclusive `hw:CARD=FC3,DEV=0` (`S32_LE`, 48 kHz, 96 frames $\times$ 2 periods / 2.00 ms) with PipeWire and PulseAudio excised from the rootfs, and arbitrated cleanly against `FLAC_AX` (`flac_ax` / `ax_av_studio`) via `/run/lock/ax-dac.lock`.
  * **Secondary Fallback:** Retains `CubebSink` and `SDLSink` as automatic failover endpoints while still capturing live `SinkStream` jitter and underrun telemetry.
* **Zero-Heap Graph Compilation & Cache-Aligned `HardwareOpus`:** Carves memory spans via `WorkbufferAllocator::Allocate<T>` with C++20 `std::construct_at`; injects `CommandMagic = 0xCAFEBABE`; uses polynomial time estimation (`ICommandProcessingTimeEstimator`) with priority voice dropping; and aligns `HardwareOpus` mailboxes to 64-byte cache lines.

---

## 🎬 Section 5: Real-Time Video, Telemetry OSD, Storage/Shader Quarantine, GTX 980 Ti Texture Compression Policy, & Telemetry

**Component Cluster:** `vk_scheduler.cpp`, `telemetry_osd.cpp/.h`, `vk_pipeline_worker.h/.cpp`, `vk_pipeline_cache.cpp`, `vk_cache_decompression.cpp`, `vk_ubo_passthrough.h/.cpp`, `vk_texture_cache.cpp`, `texture_cache.h`, `image_base.cpp`, `vulkan_device.cpp`, `vk_swapchain.h/.cpp`, `ncz_decompressor.cpp`, `vp8.h/.cpp`, `vp9.h/.cpp`, `codec_types.h`, `nvdec.h`, `qsv_decoder.h/.cpp`, `vaapi_dmabuf_bridge.hpp`, `perf_stats.h/.cpp`, `reporter.h/.cpp`, `shared_translation.cpp`, `main.cpp`, `Scripts\macro_update.ps1`, `Scripts\shader_validate.ps1`, `%AX_TOOLS_DIR%\track_telemetry.ps1`

### 🔍 Microarchitectural Status Vetting & Technical Reality

* **Automated Shader Macro Harmonization & Pre-Build SPIR-V Validation (`Scripts\macro_update.ps1` & `Scripts\shader_validate.ps1`):** Enforces explicit numeric macro defines (`USE_EASU`, `USE_RCAS`, `YUZU_USE_FP16`, `VERSION`) across all shader wrappers, standardises all GLSL headers to `#version 460 core` with `#extension GL_GOOGLE_include_directive`, and runs `glslangValidator` pre-build checks to catch Vulkan SPIR-V validation errors before C++ compilation.
* **Shared-Memory Live Telemetry Bridge & Regression Gate (`ax::telemetry`, `%AX_TOOLS_DIR%\track_telemetry.ps1` & `check_regression.ps1`):** Maps 64-byte cache-line-padded atomic counters (`ax::TelemetryCounters`) into `Local\AX_Telemetry` (Windows) and `/dev/shm/ax_telemetry` (Linux) via `ax::InitTelemetryShm()`. `track_telemetry.ps1` samples live C++20 shared memory (`VramUsed_GiB`, `VramReaps`, `Bar1Stalls`, `IgpuOffloads`, `AudioJitterMin_us`, `AudioJitterMax_us`, `AudioUnderruns`) with zero disk-scraping overhead (falling back to log parsing only when SHM is inactive) and outputs `build_stage_telemetry_metrics.json` for `check_regression.ps1`.
* **Native Vulkan Telemetry OSD & SPIR-V Pipeline (`telemetry_osd.cpp`):** Integrates real-time RAM working set and commit memory monitoring via Win32 `psapi` (and `ax::GetTelemetrySnapshot()`) directly into the engine render loop, rendering text overlays using compiled binary SPIR-V font shaders (`font.vert.spv`, `font.frag.spv`) through custom Graphics Pipeline State Objects (PSOs).
* **Vulkan Query Pools & Pipeline Statistics:** Leverages native `VK_QUERY_TYPE_PIPELINE_STATISTICS` query pools to track vertex, primitive, and shader invocation metrics asynchronously without stalling the GPU command stream—providing the gameplay-activity signal for closed-loop **Telemetry-Guided PGO** qualification.
* **Dedicated Vulkan Command Submission Ring & Zero-Allocation Descriptors:** Locks `VulkanWorker` onto Core 1 (`VulkanSubmit = 0x02`) at `ThreadPriority::VeryHigh` alongside the 1:1 GTX 980 Ti MSI interrupt (`0x02`) on its dedicated 4.8 GHz pipeline, routing uniform updates through `DescriptorUpdateManager` (`vkCreateDescriptorUpdateTemplate`).
  * Clamps `VkImageSubresourceRange` mip/layer counts prior to `vkCreateImageView` and validates sub-buffer alignment limits within `vk_ubo_passthrough.cpp` to eliminate `VK_ERROR_DEVICE_LOST` faults.
  * Calibrated 3D compute unswizzle tiers (`gpu_unswizzle_max_texture_size`: 64/128/256 MiB, `stream_size`: 8/16/32 MiB, `chunk_size`: 64/128/256 slices).
* **Storage & Auxiliary Quarantine Hierarchy on Core 5 (`0x20`):**
  * **Level-18 `.chd` & `ScopedNczQuarantine` Decompression (`ncz_decompressor.cpp` — `BelowNormal`):** Temporarily migrates ZSTD decompression threads onto Core 5 (`0x20`) at `THREAD_PRIORITY_BELOW_NORMAL`.
  * **Single-Worker SPIR-V Shader Compiler Ring (`vk_pipeline_worker.h/.cpp` & `vk_pipeline_cache.cpp` — `Low`):** Enforces `ASYNC_WORKER_CORE_CLAMP = 1` strictly on `0x20` (Core 5) with `WaitForRequests` drain synchronisation and 32-byte aligned AVX2 non-temporal pipeline cache decompression.
  * **Main GUI / Controller Input / USB MIDI Loop (`main.cpp` — `Normal`):** Pins the Win32 message pump, controller polling, and background Core 5 reaper to Core 5 (`0x20`).
* **GTX 980 Ti Texture Compression & Hybrid Policy (Option C — Active + Roadmap Target):**
  * **Why Compression Policy Matters on Maxwell GM200:** The GTX 980 Ti (6 GB GDDR5) lacks native hardware ASTC decoding. Decoding ASTC into Uncompressed (`RGBA8`) preserves 100% lossless visuals but expands VRAM footprint by $4\times\text{--}16\times$, risking breaches of your 5.50 GB VRAM ceiling.
  * **Active Option C Hybrid Runtime Policy:** Keep ASTC Recompression set to Uncompressed (`RGBA8`) for titles staying under $\sim 4.5\text{ GB}$ VRAM; switch to BC3 only on VRAM-heavy titles or when running $2\times$/$3\times$ resolution scaling.
  * **Roadmap Target (Phase 3):** Add hardware-native BC7 (`VK_FORMAT_BC7_UNORM_BLOCK`) and BC6H (`VK_FORMAT_BC6H_UFLOAT_BLOCK` for HDR skyboxes) dispatched onto Intel UHD 630's 24 native-FP16 EUs via `dma_buf` / NT Shared Handle (`astc_to_bc7.comp`).
* **Intel UHD 630 Silicon Offloading:** Offloads H.264/HEVC/VP8/VP9 decoding onto the Intel UHD Graphics 630 iGPU via Windows COM `IDXGIResource1` NT Shared Handles or Piccolo `intel-media-driver` VA-API planar NV12 `dma_buf` FDs (`<= 3.50 GB/s` DDR4 ring-bus guardrail).

---

## 📦 Section 6: Consolidated Upstream PR Triage & Silicon Adaptation Matrix

| Upstream PR | Source | Target Phase | Architectural Integration & Silicon Rationale |
| :--- | :--- | :--- | :--- |
| **Eden PR #4819 & #4210** | Eden | Phase 1 | 64-bit CityHash pipeline key condensation & bit-packed 32-bit `FixedPipelineState`—compresses state dictionary lookups into Skylake L1/L2 caches. |
| **Eden PR #4664** | Eden | Phase 1 | Validates reverting multi-threaded async shader races (`#2560`)—enforces `ASYNC_WORKER_CORE_CLAMP = 1` strictly on Core 5 (`0x20`) with a runtime fallback toggle (`async_shader_worker_enabled`). |
| **Eden PR #4668, #3817 & #2** | Eden | Phase 1 | Sizes `heap_tracker` virtual memory budgets from actual mapping counts alongside `SeLockMemoryPrivilege` (`#4668`/`#3817`); adds coefficient bounds checks to `BiquadFilterEffectCommand` on Core 0 (`0x01`) to prevent `SIGSEGV` faults (`#2`). |
| **Citron PR #4688, #449, #383** | Citron / Neo | Phase 2 | Hardcodes GTX 980 Ti `maxPushDescriptors` cap with null-descriptor fallbacks (`#4688`) and guards `vk_ubo_passthrough` / `dma_ring_bridge` against non-contiguous physical page DMA transfers (`#449`) and sub-aligned buffer ranges (`#383`). |
| **Citron PR #448** | Citron Neo | Phase 2 | Short-circuits index estimation in `vk_rasterizer.cpp` / `maxwell_3d.cpp` for procedural vertex-less draws (`gl_VertexIndex` / `SV_VertexID`) on Core 1 (`0x02`). |
| **Citron PR #447 & #385** | Citron Neo | Phase 2 & 3 | Clamps `VkImageSubresourceRange` mip/layer bounds in `vk_texture_cache.cpp` (`#385`) and validates SPIR-V descriptor types (`#447`) to prevent Maxwell GM200 `VK_ERROR_DEVICE_LOST` faults. |

| **Citron PR #4966, #5311, #5102** | Citron | Phase 3 | Injects formatless image bitmask validation (`#4966`) for Maxwell GM200 and routes async query cache recycling onto the Core 5 (`0x20`) worker ring during VBLANK (`#5102` / `#5311`). |
| **Eden PR #32** | Eden | Phase 3 | Uses CPU address deltas in `texture_cache.h` for overlap invalidation decisions—cutting false-positive `GPUDirtyMemoryManager` flushes on Core 1 (`0x02`) and reducing 6 GB VRAM churn. |

---

## 🗺️ Section 7: Expanded 6-Phase Linear Engineering Roadmap

### 🎨 Phase 1: Compiler Stability, Core Partitioning & Instrumentation Foundation (`v1.33` → `v1.34`)
* **Focus:** Stabilise execution timing, enforce cache-sticky core partitioning, harden Core 0/5 boundaries, automate shader macro normalization/SPIR-V validation, and deploy live per-core telemetry before attempting advanced GPU concurrency.
* **File Footprint:** `CMakeLists.txt`, `eden_engine.ini`, `PGO_stage0LD.bat`, `PGO_stage1.bat`, `PGO_stage2.bat`, `Scripts\macro_update.ps1`, `Scripts\shader_validate.ps1`, `Scripts\telemetry_daemon.ps1`, `src/common/thread.h/.cpp`, `src/common/bmi_util.h`, `src/common/settings_enums.h`, `src/core/cpu_manager.cpp`, `src/core/core_timing.cpp`, `src/core/memory.cpp`, `src/core/crypto/ncz_decompressor.cpp`, `src/audio_core/renderer/system_manager.cpp`, `src/video_core/renderer_vulkan/vk_pipeline_worker.h/.cpp`, `vk_pipeline_cache.cpp`, `vk_cache_decompression.cpp`, `src/video_core/host1x/codecs/` (`nvdec.h`, `vp8.cpp`, `vp9.cpp`)
* **Key Activities:** Elastic `2:3:1` core partitioning across uniform 4.8 GHz clock blocks, single-worker Core 5 shader pool & FFmpeg slice quarantine, audio biquad coefficient hardening, upstream hash condensation, numeric shader macro harmonization (`macro_update.ps1`), pre-build `glslangValidator` checks (`shader_validate.ps1`), and async JSON telemetry profiling (`telemetry_daemon.ps1`).

### 🎨 Phase 2: Descriptor Throughput, Contiguity Safety & Math Acceleration (`v1.34`)
* **Focus:** Maximise GPU/CPU arithmetic execution speed, harden Core 0 resampling math, and eliminate CPU-side descriptor pool validation overhead on Core 1 (`0x02`) without compromising Maxwell GM200 memory safety.
* **File Footprint:** `src/video_core/macro/macro.h/.cpp`, `macro_jit_compiler.h/.cpp`, `src/video_core/engines/maxwell_3d.cpp`, `src/video_core/renderer_vulkan/vk_rasterizer.cpp`, `vk_ubo_passthrough.h/.cpp`, `vk_update_descriptor.h/.cpp`, `vk_texture_cache.cpp`, `emit_spirv.cpp`, `dma_ring_bridge.h/.cpp`, `src/audio_core/sink/flex_asio_manager.cpp`
* **Key Activities:** Spatially partitioned macro-JIT core, page-contiguity-guarded push descriptor streams, zero-vertex-stream procedural draw fastpath, and Maxwell image view range clamping.

### 🎨 Phase 3: Async `5.00 GiB` VRAM Shield, Dynamic `2:3:1` Topology, SHM Telemetry & BC7/BC6H Compression (`v1.35` – `v1.35.1`)
* **Focus:** Eliminate VBLANK VRAM eviction stalls within the GTX 980 Ti's 6 GB budget via proactive `5.00 GiB` `VK_QUEUE_TRANSFER_BIT` + Vulkan 1.4 split-barrier pruning, deploy dynamic P-core/SMT `2:3:1` scaling + batch SPSC audio rings + shared-memory telemetry (`v1_36_1`) via `ax-sdk`, qualify Stage 1 PGO sweeps via live telemetry, and upgrade texture compression to BC7/BC6H.
* **File Footprint:** `ax-sdk/include/ax/{vram,topology,audio,telemetry,config}.hpp`, `ax-sdk/src/{topology,telemetry,config}.cpp`, `vk_rasterizer.h/.cpp`, `vk_vram_shield.hpp`, `vulkan_device.h/.cpp`, `vulkan_wrapper.h/.cpp`, `lockfree_audio_queue.hpp`, `ax_engine.ini`, `texture_cache.*`, `astc_to_bc7.comp`
* **Key Activities:** **`[COMPLETED]`** Proactive `5.00 GiB` async VRAM shield (`VkVramShield` + `VK_QUEUE_TRANSFER_BIT` + `vkCmdSetEvent2`/`WaitEvents2`), dynamic `GetLogicalProcessorInformationEx` / `sysfs` `2:3:1` physical-core scaler, cache-line-padded batch SPSC audio queue, cross-process shared-memory telemetry (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`), and 7-repository ecosystem convergence (`Generation-X-AX`, `FLAC_AX`, `Piccolo-AX`, `piccolo-ax-desktop`); **`[REMAINING]`** Telemetry-Guided PGO sweep qualification, high-fidelity BC7/BC6H compute transcode shader (`astc_to_bc7.comp`), and CPU-delta overlap invalidation (`PR #32`).

### 🎨 Phase 4: Async Concurrency, Cross-Silicon Offloading & Reversible IRQ Steering (`v1.36`)
* **Focus:** Leverage parallel hardware pipelines (GTX 980 Ti Async Compute, Intel UHD 630 iGPU, WinRT WGC / `lsfg-vk 2.0.0+`, and Vulkan Video Decode) without PCIe bottlenecks, and replace manual Windows registry modifications with a reversible runtime MSI IRQ helper.
* **File Footprint:** `vk_device.h/.cpp`, `vk_async_compute.hpp`, `vk_staging_buffer_pool.cpp`, `vk_synchronization2.hpp`, `vk_memory_manager.cpp`, `vk_wgc_capture.cpp`, `qsv_decoder.h/.cpp`, `vaapi_dmabuf_bridge.hpp`, `ax-sdk/tools/set_irq_affinity.ps1`
* **Key Activities:** Zero-copy WinRT WGC capture, Vulkan asynchronous compute, insulated Intel UHD 630 QSV / VA-API bridge, and reversible runtime MSI IRQ helper targeted to flat 4.8 GHz profiles.

### 🎨 Phase 5: Closed-Loop Telemetry, Predictive Storage Pre-Faulting & Production Appliance (`v1.37`)
* **Focus:** Eliminate mechanical HDD seek stalls, cut input-to-photon latency, prevent Maxwell shader register spilling, enable zero-overhead hardware capture, and ship the production Piccolo `[AX]` `v1.0` appliance.
* **File Footprint:** `vfs_prefetch_ring.h/.cpp`, `ncz_decompressor.cpp`, `core_timing.cpp`, `speed_limiter.h/.cpp`, `raw_hid_poller.h/.cpp`, `emit_spirv.cpp`, `vk_wgc_capture.cpp`, `ax-sdk/tools/ax_top.cpp`, `verify_ax_appliance.sh`
* **Key Activities:** Predictive HDD-to-NVMe block pre-faulting ring, closed-loop thermal & frame-pacing governor calibrated for symmetrical thread frequencies, Maxwell register-pressure guard, universal UAC2 USB DAC auto-probe, zero-copy ALSA `MMAP` / WaveRT ring, and Core 5 direct USB HID/MIDI poller / production appliance bake.

### 🎨 Phase 6: Blue-Sky Autonomous Silicon & Neural Runtime (`v2.00` — Forward-Looking R&D)
* **Focus:** Transform the emulator from a static translator into a self-recompiling, zero-latency hardware appliance using live superblock re-compilation, hardware optical flow on the idle Intel UHD 630 iGPU, and custom USB microcontroller hardware interrupts.
* **File Footprint:** `live_reopt_daemon.h/.cpp`, `igpu_optical_flow.h/.cpp`, `vfs_predictive_migrator.h/.cpp`, `firmware/usb_sync_bridge/`
* **Key Activities:** Live background superblock re-compilation, cross-silicon VME optical flow motion vector synthesis, custom USB microcontroller VBLANK & audio clock-sync dongle, and predictive NVMe #2 whole-title hot-migration.
